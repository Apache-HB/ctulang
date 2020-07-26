#ifndef CTHULHU_H
#define CTHULHU_H

#include <stdint.h>
#include <stddef.h>

typedef int(*CtNextFunc)(void*);

typedef struct {
    char *ptr;
    size_t len;
    size_t alloc;
} CtBuffer;

typedef struct {
    struct CtState *source;
    size_t dist;
    size_t col;
    size_t line;
} CtOffset;

typedef enum {
#define KEY(id, str, flags) id,
#define OP(id, str) id,
#include "keys.inc"

    K_INVALID
} CtKey;

typedef struct {
    size_t offset;
    size_t len;
} CtView;

typedef struct {
    size_t offset;
    size_t len;

    /* TODO: optimize this */
    int multiline;
} CtString;

typedef struct {
    enum { BASE2, BASE10, BASE16 } enc;
    size_t num;
    CtView suffix;
} CtDigit;

typedef struct {
    enum {
        TK_IDENT,
        TK_KEY,
        TK_INT,
        TK_STRING,
        TK_CHAR,
        TK_END,

        TK_INVALID
    } type;

    union {
        CtView ident;
        CtString str;
        size_t letter;
        CtKey key;
        CtDigit digit;
    } data;

    CtOffset pos;
    size_t len;
} CtToken;

typedef enum {
    ERR_NONE = 0,

    /* non-fatal */

    /* integer literal was too large to fit into max size */
    ERR_OVERFLOW,

    /* invalid escape sequence in string/char literal */
    ERR_INVALID_ESCAPE,

    /* a linebreak was found inside a single line string */
    ERR_STRING_LINEBREAK,



    /* fatal */

    /* invalid character found while lexing */
    ERR_INVALID_SYMBOL,

    /* the EOF was found while lexing a string */
    ERR_STRING_EOF,

    /* the closing ' was missing while parsing a char literal */
    ERR_CHAR_CLOSING,

    /* an unexpected keyword was encountered while parsing */
    ERR_UNEXPECTED_KEY,

    /* missing closing ) */
    ERR_MISSING_BRACE,

    /* unexpected token type */
    ERR_UNEXPECTED_TOK
} CtErrorKind;

typedef struct {
    CtErrorKind type;
    CtOffset pos;
    size_t len;

    /* associated token */
    CtToken tok;
} CtError;


typedef enum {
    AK_OTHER,
    AK_IDENT,

    AK_BINARY,
    AK_UNARY,
    AK_TERNARY,
    AK_LITERAL,
    AK_INIT,
    AK_ARG,
    AK_NAME,
    AK_CALL,
    AK_SUB,
    AK_ACCESS,
    AK_DEREF,

    AK_PTR,
    AK_CLOSURE,
    AK_ARRAY,
    AK_QUAL,
    AK_QUALS,
    AK_PARAM,

    AK_STMTS,
    AK_FUNC,
    AK_ARGDECL,
    AK_CAPTURE
} CtASTKind;

typedef struct {
    struct CtAST *nodes;
    size_t len;
    size_t alloc;
} CtASTArray;

typedef struct CtAST {
    CtASTKind type;
    CtToken tok;

    union {
        struct {
            struct CtAST *lhs;
            struct CtAST *rhs;
        } binary;

        struct {
            struct CtAST *cond;
            struct CtAST *yes;
            struct CtAST *no;
        } ternary;

        struct CtAST *expr;

        struct CtAST *ptr;

        struct {
            struct CtAST *type;
            struct CtAST *size;
        } array;

        CtASTArray quals;

        struct {
            struct CtAST *name;
            CtASTArray params;
        } qual;

        struct {
            struct CtAST *name;
            struct CtAST *type;
        } param;

        struct {
            CtASTArray args;
            struct CtAST *result;
        } closure;

        CtASTArray stmts;

        CtASTArray args;

        struct {
            struct CtAST *field;
            struct CtAST *expr;
        } arg;

        struct {
            struct CtAST *name;
            struct CtAST *init;
        } name;

        struct {
            struct CtAST *expr;
            CtASTArray args;
        } call;

        struct {
            struct CtAST *expr;
            struct CtAST *index;
        } sub;

        struct {
            struct CtAST *expr;
            struct CtAST *field;
        } access;

        struct {
            struct CtAST *expr;
            struct CtAST *field;
        } deref;

        struct {
            struct CtAST *name;
            struct CtAST *type;
            struct CtAST *init;
        } argdecl;

        struct {
            struct CtAST *name;
            CtASTArray args;
            CtASTArray captures;
            struct CtAST *result;
            struct CtAST *body;
        } func;

        struct {
            struct CtAST *symbol;
            int ref;
        } capture;
    } data;
} CtAST;

typedef struct CtState {
    /* stream state */
    const char *name;
    void *stream;
    CtNextFunc next;
    int ahead;
    CtBuffer source;

    /* lexing state */
    CtOffset pos;
    size_t len;
    CtBuffer strings;

    enum {
#define FLAG(name, bit) name = (1 << bit),
#include "keys.inc"
        LF_DEFAULT = LF_CORE
    } flags;

    int depth;

    /* error handling state */
    CtError lerr;
    CtError perr;

    CtError *errs;
    size_t err_idx;
    size_t max_errs;

    /* parsing state */
    CtToken tok;

    /* constants */
    CtAST *empty;
} CtState;

void ctStateNew(
    CtState *self,
    void *stream,
    CtNextFunc next,
    const char *name,
    size_t max_errs
);

/* interpreter style parsing. TODO: lots of callbacks for this one */
CtAST *ctParseInterp(CtState *self);

/* traditional parsing. TODO: also callbacks needed */
CtAST *ctParse(CtState *self);

#endif /* CTHULHU_H */
