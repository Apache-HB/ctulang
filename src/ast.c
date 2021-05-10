#include "ast.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "bison.h"

static node_t*
new_node(node_kind_t kind) 
{
    node_t *node = malloc(sizeof(node_t));
    node->kind = kind;
    node->mut = false;
    node->exported = false;
    node->where = NULL;
    return node;
}

static node_t*
new_decl(node_kind_t kind, char *name)
{
    node_t *node = new_node(kind);
    node->decl.name = name;
    node->decl.attribs = NULL;
    return node;
}

static nodes_t*
new_nodes(size_t size)
{
    nodes_t *nodes = malloc(sizeof(nodes_t));
    nodes->length = 0;
    nodes->size = size;
    nodes->data = malloc(sizeof(node_t) * size);
    return nodes;
}

node_t*
add_loc(node_t *node, YYLTYPE *lloc)
{
    node->where = malloc(sizeof(YYLTYPE));
    memcpy(node->where, lloc, sizeof(YYLTYPE));
    return node;
}

nodes_t*
empty_node_list(void)
{
    nodes_t *nodes = new_nodes(4);
    return nodes;
}

nodes_t*
new_node_list(node_t *init)
{
    nodes_t *nodes = new_nodes(4);
    nodes = node_append(nodes, init);
    return nodes;
}

nodes_t*
node_append(nodes_t *self, node_t *node)
{
    if (self->length + 1 >= self->size) {
        self->size += 8;
        self->data = realloc(self->data, sizeof(node_t) * self->size);
    }
    memcpy(self->data + self->length, node, sizeof(node_t));
    self->length += 1;
    return self;
}

nodes_t*
node_replace(nodes_t *self, size_t idx, node_t *node)
{
    memcpy(self->data + idx, node, sizeof(node_t));
    return self;
}

nodes_t*
node_prepend(nodes_t *self, node_t *node)
{
    if (self->length + 1 >= self->size) {
        self->size += 8;
        self->data = realloc(self->data, sizeof(node_t) * self->size);
    }
    memmove(self->data + 1, self->data, sizeof(node_t) * self->length);
    memcpy(self->data, node, sizeof(node_t));
    self->length += 1;
    return self;
}

nodes_t*
nodes_merge(nodes_t *self, nodes_t *other)
{
    size_t len = self->length + other->length;
    if (len >= self->size) {
        self->size = len;
        self->data = realloc(self->data, sizeof(node_t) * self->size);
    }
    memcpy(self->data + self->length, other->data, sizeof(node_t) * other->length);
    self->length = len;
    return self;
}

node_t*
set_exported(node_t *decl, bool exported)
{
    decl->exported = exported;
    return decl;
}

node_t*
add_attribs(node_t *decl, nodes_t *attribs)
{
    decl->decl.attribs = attribs;
    return decl;
}

node_t*
new_digit(char *digit, char *suffix) 
{
    node_t *node = new_node(NODE_DIGIT);
    node->digit.digit = digit;
    node->digit.suffix = suffix;

    if (strncmp(digit, "0x", 2) == 0) {
        node->digit.base = 16;
        node->digit.digit += 2;
    } else {
        node->digit.base = 10;
    }
    return node;
}

node_t*
new_arg(char *name, node_t *expr)
{
    node_t *node = new_node(NODE_ARG);
    node->arg.name = name;
    node->arg.expr = expr;
    return node;
}

node_t*
new_attrib(char *name, nodes_t *args)
{
    node_t *node = new_node(NODE_ATTRIB);
    node->decl.name = name;
    node->decl.args = args;
    return node;
}

node_t*
new_unary(unary_op_t op, node_t *expr) 
{
    node_t *node = new_node(NODE_UNARY);
    node->unary.op = op;
    node->unary.expr = expr;
    return node;
}

node_t*
new_binary(binary_op_t op, node_t *lhs, node_t *rhs) 
{
    node_t *node = new_node(NODE_BINARY);
    node->binary.op = op;
    node->binary.lhs = lhs;
    node->binary.rhs = rhs;
    return node;
}

node_t*
new_ternary(node_t *cond, node_t *yes, node_t *no) 
{
    node_t *node = new_node(NODE_TERNARY);
    node->ternary.cond = cond;
    node->ternary.yes = yes;
    node->ternary.no = no;
    return node;
}

node_t*
new_call(node_t *body, nodes_t *args)
{
    node_t *node = new_node(NODE_CALL);
    node->call.body = body;
    node->call.args = args;
    return node;
}

node_t*
new_return(node_t *expr)
{
    node_t *node = new_node(NODE_RETURN);
    node->expr = expr;
    return node;
}

node_t*
new_func(char *name, nodes_t *params, node_t *result, node_t *body)
{
    node_t *node = new_decl(NODE_FUNC, name);
    node->decl.func.params = params;
    node->decl.func.result = result;
    node->decl.func.body = body;
    return node;
}

node_t*
new_param(char *name, node_t *type)
{
    node_t *node = new_decl(NODE_PARAM, name);
    node->decl.param = type;
    return node;
}

node_t*
new_name(char *name)
{
    node_t *node = new_node(NODE_NAME);
    node->name = name;
    return node;
}

node_t*
new_typename(char *name)
{
    node_t *node = new_node(NODE_TYPENAME);
    node->name = name;
    return node;
}

node_t*
new_builtin_type(
    const char *name, builtin_kind_t kind, 
    int width, const char *cname
)
{
    node_t *node = new_decl(NODE_BUILTIN_TYPE, strdup(name));
    node->decl.builtin.kind = kind;
    node->decl.builtin.width = width;
    node->decl.builtin.cname = cname;
    return node;
}

node_t*
new_var(char *name, node_t *type, node_t *init, bool mut)
{
    node_t *node = new_decl(NODE_VAR, name);
    node->mut = mut;
    node->decl.var.type = type;
    node->decl.var.init = init;
    return node;
}

node_t*
new_assign(node_t *old, node_t *expr)
{
    node_t *node = new_node(NODE_ASSIGN);
    node->assign.old = old;
    node->assign.expr = expr;
    return node;
}

node_t*
new_pointer(node_t *type)
{
    node_t *node = new_node(NODE_POINTER);
    node->type = type;
    return node;
}

node_t*
new_compound(nodes_t *nodes)
{
    node_t *node = new_node(NODE_COMPOUND);
    node->compound = nodes;
    return node;
}

node_t*
new_closure(nodes_t *args, node_t *result)
{
    node_t *node = new_node(NODE_CLOSURE);
    node->closure.args = args;
    node->closure.result = result;
    return node;
}

node_t*
new_bool(bool val)
{
    node_t *node = new_node(NODE_BOOL);
    node->boolean = val;
    return node;
}

node_t*
new_string(char *text)
{
    node_t *node = new_node(NODE_STRING);
    node->text = text;
    return node;
}

node_t*
new_access(node_t *expr, char *field)
{
    node_t *node = new_node(NODE_ACCESS);
    node->access.expr = expr;
    node->access.field = field;
    return node;
}

node_t*
new_branch(node_t *cond, node_t *body, node_t *next)
{
    node_t *node = new_node(NODE_BRANCH);
    node->branch.cond = cond;
    node->branch.body = body;
    node->branch.next = next;
    return node;
}

node_t*
new_record(char *name, nodes_t *fields)
{
    node_t *node = new_decl(NODE_RECORD, name);
    node->decl.fields = fields;
    return node;
}

node_t*
new_while(node_t *cond, node_t *body)
{
    node_t *node = new_node(NODE_WHILE);
    node->loop.cond = cond;
    node->loop.body = body;
    return node;
}

node_t*
new_break()
{
    return new_node(NODE_BREAK);
}

node_t*
new_continue()
{
    return new_node(NODE_CONTINUE);
}

node_t*
new_mut(node_t *node)
{
    node_t *out = new_node(0);
    memcpy(out, node, sizeof(node_t));
    out->mut = 1;
    return out;
}

node_t*
new_multi_string(char *text)
{
    node_t *out = new_node(NODE_STRING);
    size_t len = strlen(text);
    text[len - 2] = '\0';
    text = text + 2;
    out->text = text;
    return out;
}

node_t*
new_qual(char *scope, char *name)
{
    node_t *out = new_node(NODE_QUAL);
    out->qual.scope = scope;
    out->qual.name = name;
    return out;
}

node_t*
new_cast(node_t *expr, node_t *type)
{
    node_t *out = new_node(NODE_CAST);
    out->cast.expr = expr;
    out->cast.type = type;
    return out;
}

node_t*
new_array(node_t *type, node_t *size)
{
    node_t *out = new_node(NODE_ARRAY);
    out->array.type = type;
    out->array.size = size;
    return out;
}

node_t*
new_null()
{
    return new_node(NODE_NULL);
}

static void
dump_nodes(nodes_t *nodes)
{
    for (size_t i = 0; i < nodes->length; i++) {
        if (i != 0) {
            printf(" ");
        }
        dump_node(nodes->data + i);
    }
}

static const char*
dump_mut(bool i)
{
    return i ? "mut" : "const";
}

static const char*
dump_export(bool i)
{
    return i ? "public" : "private";
}

void
dump_node(node_t *node)
{
    switch (node->kind) {
    case NODE_DIGIT: 
        printf("(%s %d)", node->digit.digit, node->digit.base); 
        break;
    case NODE_UNARY:    
        printf("("); 
        switch (node->unary.op) {
        case UNARY_ABS: printf("abs "); break;
        case UNARY_NEG: printf("neg "); break;
        case UNARY_REF: printf("ref "); break;
        case UNARY_DEREF: printf("deref "); break;
        case UNARY_NOT: printf("not "); break;
        } 
        dump_node(node->unary.expr);
        printf(")");
        break;
    case NODE_BINARY: 
        printf("(");
        switch (node->binary.op) {
        case BINARY_ADD: printf("add "); break;
        case BINARY_SUB: printf("sub "); break;
        case BINARY_DIV: printf("div "); break;
        case BINARY_MUL: printf("mul "); break;
        case BINARY_REM: printf("rem "); break;
        }
        dump_node(node->binary.lhs);
        printf(" ");
        dump_node(node->binary.rhs);
        printf(")");
        break;
    case NODE_TERNARY:
        printf("(ternary ");
        dump_node(node->ternary.cond);
        printf(" then ");
        dump_node(node->ternary.yes);
        printf(" else ");
        dump_node(node->ternary.no);
        printf(")");
        break;
    case NODE_CALL:
        printf("(call ");
        dump_node(node->call.body);
        if (node->call.args->length) {
            printf(" ");
            dump_nodes(node->call.args);
        }
        printf(")");
        break;
    case NODE_FUNC:
        printf("(def %s %s (", dump_export(node->exported), node->decl.name);
        dump_nodes(node->decl.func.params);
        printf(") ");
        if (node->decl.func.result) {
            dump_node(node->decl.func.result);
            printf(" ");
        }
        dump_node(node->decl.func.body);
        printf(")");
        break;
    case NODE_COMPOUND:
        printf("(");
        dump_nodes(node->compound);
        printf(")");
        break;
    case NODE_NAME:
        printf("%s", node->name);
        break;
    case NODE_RETURN:
        if (node->expr) {
            printf("(return ");
            dump_node(node->expr);
            printf(")");
        } else {
            printf("return");
        }
        break;
    case NODE_TYPENAME:
        printf("(typename %s)", node->name);
        break;
    case NODE_POINTER:
        printf("(ptr ");
        dump_node(node->type);
        printf(")");
        break;
    case NODE_PARAM:
        printf("(%s ", node->decl.name);
        dump_node(node->decl.param);
        printf(")");
        break;
    case NODE_BUILTIN_TYPE:
        printf("(builtin %s)", node->decl.name);
        break;
    case NODE_VAR:
        printf("(var %s %s %s", dump_export(node->exported), dump_mut(node->mut), node->decl.name);
        if (node->decl.var.type) {
            printf(" ");
            dump_node(node->decl.var.type);
        }
        if (node->decl.var.init) {
            printf(" ");
            dump_node(node->decl.var.init);
        }
        printf(")");
        break;
    case NODE_CLOSURE:
        printf("(closure ");
        dump_node(node->closure.result);
        printf(" (");
        dump_nodes(node->closure.args);
        printf(")");
        break;
    case NODE_ASSIGN:
        printf("(assign ");
        dump_node(node->assign.old);
        printf(" ");
        dump_node(node->assign.expr);
        printf(")");
        break;
    case NODE_BOOL:
        printf("%s", node->boolean ? "true" : "false");
        break;
    case NODE_STRING:
        printf("%s", node->text);
        break;
    case NODE_ACCESS:
        printf("(");
        dump_node(node->access.expr);
        printf(" %s)", node->access.field);
        break;
    case NODE_BRANCH:
        if (node->branch.cond) {
            printf("(if ");
            dump_node(node->branch.cond);
            printf(" then ");
        }
        dump_node(node->branch.body);

        if (node->branch.next) {
            printf(" else ");
            dump_node(node->branch.next);
        }

        printf(")");
        break;
    case NODE_RECORD:
        printf("(record %s (", node->decl.name);
        dump_nodes(node->decl.fields);
        printf("))");
        break;
    case NODE_WHILE:
        printf("(while ");
        dump_node(node->loop.cond);
        printf(" do ");
        dump_node(node->loop.body);
        printf(")");
        break;
    case NODE_BREAK:
        printf("break");
        break;
    case NODE_CONTINUE:
        printf("continue");
        break;
    case NODE_QUAL:
        printf("(qual %s::%s)", node->qual.scope, node->qual.name);
        break;
    case NODE_CAST:
        printf("(cast ");
        dump_node(node->cast.expr);
        printf(" to ");
        dump_node(node->cast.type);
        printf(")");
        break;
    case NODE_ATTRIB:
        printf("(attrib %s ", node->decl.name);
        dump_nodes(node->decl.args);
        printf(")");
        break;
    case NODE_ARRAY:
        printf("(array ");
        dump_node(node->array.type);
        if (node->array.size) {
            printf(" ");
            dump_node(node->array.size);
        }
        printf(")");
        break;
    case NODE_ARG:
        printf("(arg ");
        if (node->arg.name) {
            printf("%s ", node->arg.name);
        }
        dump_node(node->arg.expr);
        printf(")");
        break;
    case NODE_NULL:
        printf("null");
        break;
    }
}
