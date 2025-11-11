#ifndef AST_H
#define AST_H

#include <stddef.h>
#include <stdbool.h>

/* Span fuente para diagnósticos */
typedef struct {
    int line_start, col_start;
    int line_end,   col_end;
} SrcSpan;

/* Kinds de nodo (extensible) */
typedef enum {
    AST_PROGRAM,        // programa raíz
    AST_BLOCK,          // bloque de código
    AST_VAR_DECL,       // declaración de variable
    AST_FUNC_DECL,      // declaración de función
    AST_PARAM,          // declaración de parámetro de función
    AST_IF,             // sentencia if
    AST_WHILE,          // sentencia while
    AST_FOR,            // sentencia for
    AST_RETURN,         // sentencia return
    AST_PRINT,          // sentencia print
    AST_READ,           // sentencia read
    AST_EXPR,           // expresión
    /* Expresiones */
    AST_IDENT,          // identificador
    AST_LITERAL,        // literal (int, float, string, bool, null)
    AST_ASSIGN,         // lhs op= rhs (op puede ser =, +=, etc.)
    AST_BINOP,          // operación binaria
    AST_UNOP,           // operación unaria
    AST_CALL,           // llamada a función
    AST_INDEX,          // indexación de array/lista
    /* Errores */
    AST_ERROR           // nodo de error
} AstKind;

/* Operadores (los que usarás en Pratt) */
typedef enum {
    OP_NONE = 0,
    OP_ASSIGN, OP_ADD_ASSIGN, OP_SUB_ASSIGN, OP_MUL_ASSIGN, OP_DIV_ASSIGN,
    OP_OR, OP_AND,
    OP_EQ, OP_NEQ, OP_LT, OP_LTE, OP_GT, OP_GTE,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_NOT, OP_NEG
} OpCode;

struct AstNode; /* fwd */

typedef struct AstList {
    struct AstNode **data;
    size_t count, cap;
} AstList;

typedef struct {
    const char *lexeme;   // puntero a buffer del lexer
} AstIdent;

typedef struct {
    /* para literales simples: int/float/string/bool/null */
    enum { LIT_INT, LIT_FLOAT, LIT_STRING, LIT_BOOL, LIT_NULL } tag;
    union { long i; double f; const char *s; bool b; } val;
} AstLiteral;

/* Nodo AST */
typedef struct AstNode {
    AstKind kind;
    SrcSpan span;
    union {
        /* PROGRAM */
        struct { AstList decls; } program;

        /* BLOCK */
        struct { AstList stmts; } block;

        /* VAR_DECL */
        struct { const char *name; const char *type_name; struct AstNode *init; } var_decl;

        /* FUNC_DECL */
        struct { const char *name; AstList params; const char *ret_type; struct AstNode *body; } func_decl;

        /* PARAM */
        struct { const char *name; const char *type_name; } param;

        /* IF */
        struct { struct AstNode *cond; struct AstNode *then_blk; AstList elseifs; struct AstNode *else_blk; } if_stmt;

        /* WHILE */
        struct { struct AstNode *cond; struct AstNode *body; } while_stmt;

        /* FOR (según tu sintaxis) */
        struct { struct AstNode *init; struct AstNode *cond; struct AstNode *step; struct AstNode *body; } for_stmt;

        /* RETURN */
        struct { struct AstNode *expr; } ret;

        /* PRINT */
        struct { AstList exprs; } print;

        /* READ */
        struct { AstList targets; } read;

        /* EXPR */
        struct { struct AstNode *expr; } expr;

        /* ASSIGN */
        struct { OpCode op; struct AstNode *lhs; struct AstNode *rhs; } assign;

        /* BINOP / UNOP */
        struct { OpCode op; struct AstNode *left; struct AstNode *right; } binop;
        struct { OpCode op; struct AstNode *expr; } unop;

        /* CALL */
        struct { struct AstNode *callee; AstList args; } call;

        /* INDEX */
        struct { struct AstNode *base; struct AstNode *index; } index;

        /* ERROR */
        struct { const char *msg; } error;

        /* IDENT / LITERAL */
        AstIdent ident;
        AstLiteral literal;
    } as;
} AstNode;

/* API de creación/gestión */
typedef struct AstArena AstArena;

AstArena *ast_arena_new(void);
void      ast_arena_free(AstArena *a);

AstNode  *ast_new(AstArena *a, AstKind kind, SrcSpan span);
void      ast_list_push(AstArena *a, AstList *list, AstNode *node);

char *ast_arena_strndup(AstArena *a, const char *s, size_t n);

#endif /* AST_H */
