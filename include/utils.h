#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

typedef enum {
    TOK_IDENTIFIER,
    TOK_ARITHMETIC_OP,
    TOK_LOGIC_OP,
    TOK_KEYWORD,
    TOK_NUMBER,
    TOK_LITERAL,
    TOK_FLOAT,
    TOK_STRING,
    TOK_CHAR,
    TOK_BOOLEAN,
    TOK_COMMENT,
    TOK_DELIMITER,
    TOK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    size_t length;
    size_t line;
    size_t column;
} Token;

typedef struct {
    Token *tokens;
    size_t count;
    size_t capacity;
} TokenList;

typedef struct {
    int code;
    char *lexeme;
    size_t length;
    size_t line;
    size_t column;
} ErrorToken;

// Función para manejar errores
void handleError(const ErrorToken *error);

// Función para duplicar cadenas
char *duplicateString(const char *str);

// Función para imprimir un mensaje de depuración
void debugPrint(const char *message);

char *substr_dup(const char *src, size_t start, size_t len);

#endif // UTILS_H