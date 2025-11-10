#ifndef LEXER_H
#define LEXER_H

#include "utils.h"
#include <stddef.h>
#include <stdbool.h>
#include "interner.h"

typedef struct {
    const char *source;
    size_t length;
    size_t position;
    size_t line;
    size_t column;

    Interner *I;
    Interned  S; 
} Lexer;

// Inicializa el lexer con la fuente dada
void init_lexer(Lexer *lexer, const char *source, Interner *I, const Interned *S);

// Tokeniza la fuente y llena la lista de tokens
void tokenize(Lexer *lexer, TokenList *list);

#endif 
