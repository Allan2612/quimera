#ifndef LEXER_H
#define LEXER_H

#include "utils.h"
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    const char *source;
    size_t length;
    size_t position;
    size_t line;
    size_t column;
} Lexer;

void init_lexer(Lexer *lexer, const char *source);
void tokenize(Lexer *lexer, TokenList *list);
void init_token_list(TokenList *list);
void free_token_list(TokenList *list);
void add_token(TokenList *list, Token token);

#endif // LEXER_H
