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

// Inicializa el lexer con la fuente dada
void init_lexer(Lexer *lexer, const char *source);

// Tokeniza la fuente y llena la lista de tokens
void tokenize(Lexer *lexer, TokenList *list);

#endif 
