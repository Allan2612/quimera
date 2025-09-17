#ifndef LEXER_H
#define LEXER_H

#include "utils.h"
#include <stddef.h>
#include <stdbool.h>

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
    const char *source;
    size_t length;
    size_t position;
    size_t line;
    size_t column;
} Lexer;

typedef struct {
    Token *tokens;
    size_t count;
    size_t capacity;
} TokenList;

void init_lexer(Lexer *lexer, const char *source);
void tokenize(Lexer *lexer, TokenList *list);
void init_token_list(TokenList *list);
void free_token_list(TokenList *list);
void add_token(TokenList *list, Token token);

#endif // LEXER_H
