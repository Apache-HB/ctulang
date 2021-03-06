#include "ir.h"
#include "type.h"
#include "common.h"

static const char *get_flow_name(module_t *mod, size_t idx) {
    return mod->flows[idx].name;
}

static void debug_imm(imm_t imm) {
    switch (imm.kind) {
    case IMM_INT: printf("int(%" PRId64 ")", imm.imm_int); break;
    case IMM_BOOL: printf("bool(%s)", imm.imm_bool ? "true" : "false"); break;
    case IMM_SIZE: printf("size(%zu)", imm.imm_size);
    }
}

static void debug_operand(module_t *mod, operand_t op) {
    switch (op.kind) {
    case VREG: printf("%%%zu", op.vreg); break;
    case ARG: printf("arg[%zu]", op.arg); break;
    case IMM: debug_imm(op.imm); break;
    case NONE: printf("none"); break;
    case BLOCK: printf(".%zu", op.label); break;
    case FUNC: printf("def(%zu:%s)", op.func, get_flow_name(mod, op.func)); break;
    case GLOBAL: printf("var(%zu:%s)", op.var, "TODO"); break;
    }
}

static void debug_index(size_t idx) {
    printf("  %%%zu = ", idx);
}

static void debug_args(module_t *mod, step_t step) {
    printf("(");
    for (size_t i = 0; i < step.len; i++) {
        if (i != 0) {
            printf(", ");
        }
        debug_operand(mod, step.args[i]);
    }
    printf(")");
}

static void debug_step(module_t *mod, size_t idx, step_t step) {
    switch (step.opcode) {
    case OP_EMPTY:
        return;
    case OP_RETURN:
        printf("  ret ");
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.value);
        break;
    case OP_BINARY:
        debug_index(idx); 
        debug_type(step.type);
        printf(" %s ", binary_name(step.binary));
        debug_operand(mod, step.lhs);
        printf(" ");
        debug_operand(mod, step.rhs);
        break;
    case OP_UNARY:
        debug_index(idx); 
        debug_type(step.type);
        printf(" %s ", unary_name(step.unary));
        debug_operand(mod, step.expr);
        break;
    case OP_VALUE:
        debug_index(idx); 
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.value);
        break;
    case OP_BLOCK:
        printf(".%zu:", idx);
        break;
    case OP_BRANCH:
        printf("  branch ");
        debug_operand(mod, step.cond);
        printf(" ");
        debug_operand(mod, step.block);
        printf(" else ");
        debug_operand(mod, step.other);
        break;
    case OP_CALL:
        debug_index(idx);
        printf("call ");
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.value);
        debug_args(mod, step);
        break;
    case OP_CONVERT:
        debug_index(idx);
        printf("cast ");
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.value);
        break;
    case OP_RESERVE:
        debug_index(idx);
        printf("reserve ");
        debug_type(step.type);
        break;
    case OP_LOAD:
        debug_index(idx);
        printf("load ");
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.src);
        break;
    case OP_STORE:
        printf("  store ");
        debug_type(step.type);
        printf(" ");
        debug_operand(mod, step.dst);
        printf(" ");
        debug_operand(mod, step.src);
        break;
    case OP_JUMP:
        printf("  jmp ");
        debug_operand(mod, step.block);
        break;
    }

    printf("\n");
}

static void debug_params(flow_t *flow) {
    printf("(");

    for (size_t i = 0; i < flow->nargs; i++) {
        arg_t arg = flow->args[i];
        if (i != 0) {
            printf(", ");
        }
        printf("%zu = [%s: ", i, arg.name);
        debug_type(arg.type);
        printf("]");
    }

    printf(")");
}

static void debug_flow(module_t *mod, flow_t flow) {
    printf("define %s", flow.name);
    debug_params(&flow);
    printf(": ");
    debug_type(flow.result);
    printf(" {\n");

    for (size_t i = 0; i < flow.len; i++) {
        debug_step(mod, i, flow.steps[i]);
    }
    printf("}\n");
}

void debug_module(module_t mod) {
    for (size_t i = 0; i < num_flows(&mod); i++) {
        if (i != 0) {
            printf("\n");
        }
        debug_flow(&mod, mod.flows[i]);
    }
}