#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

typedef enum {
    TOK_IDENTIFIER,
    TOK_KEYWORD,
    TOK_INTEGER,
    TOK_FLOAT,
    TOK_ARITHMETIC_OP,
    TOK_LOGIC_OP,
    TOK_ASSIGNMENT_OP,
    TOK_STRING,
    TOK_CHAR,
    TOK_BOOLEAN,
    TOK_DELIMITER,
    TOK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    const char *lexeme;
    size_t length;
    size_t line;
    size_t column;
} Token;

typedef struct {
    Token *tokens;
    size_t count;
    size_t capacity;
} TokenList;

// Función para inicializar lista de tokens
void init_token_list(TokenList *list);

// Función para inicializar un token
void init_token(Token *token);

// Función para liberar memoria de la lista de tokens
void free_token_list(TokenList *list);

// Función para agregar un token a la lista
void add_token(TokenList *list, Token token);

#endif // UTILS_H