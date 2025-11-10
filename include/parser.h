#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <stddef.h>

typedef struct {
    const Token *toks;
    size_t count;
    size_t i;            // índice actual
    AstArena *arena;
    int error_count;
} Parser;

/* Punto de entrada */
AstNode *parse_program(Parser *p);

/* Helpers de conveniencia si quieres tests por no-terminal */
AstNode *parse_block(Parser *p);   // consume hasta 'End' (no lo come)
AstNode *parse_stmt(Parser *p);
AstNode *parse_expr(Parser *p);

#endif /* PARSER_H */