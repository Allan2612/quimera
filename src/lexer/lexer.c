#include "lexer.h"
#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>

#define TOKEN_LIST_INITIAL_CAPACITY 16

void init_token_list(TokenList *list) {
    list->tokens = (Token *)malloc(TOKEN_LIST_INITIAL_CAPACITY * sizeof(Token));
    list->count = 0;
    list->capacity = TOKEN_LIST_INITIAL_CAPACITY;
}

static void resize_token_list(TokenList *list, size_t new_capacity) {
    Token *new_tokens = (Token *)realloc(list->tokens, new_capacity * sizeof(Token));
    if (new_tokens) {
        list->tokens = new_tokens;
        list->capacity = new_capacity;
    }
    // Si realloc falla, la lista original sigue válida, pero deberías manejar el error.
}

void add_token(TokenList *list, Token token) {
    if (list->count >= list->capacity) {
        resize_token_list(list, list->capacity * 2);
    }
    list->tokens[list->count++] = token;
}

void free_token_list(TokenList *list) {
    for (size_t i = 0; i < list->count; ++i) {
        free(list->tokens[i].lexeme); // Si los lexemas fueron asignados dinámicamente
    }
    free(list->tokens);
    list->tokens = NULL;
    list->count = 0;
    list->capacity = 0;
}

void init_lexer(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->length = strlen(source);
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
}

// Declaraciones de funciones especializadas (implementaciones vacías)
static void identify_identifier(Lexer *lexer, Token *token) { token->type = TOK_IDENTIFIER; }
static void identify_number(Lexer *lexer, Token *token)    { token->type = TOK_NUMBER; }
static void identify_string(Lexer *lexer, Token *token)    { token->type = TOK_STRING; }
static void identify_delimiter(Lexer *lexer, Token *token) { token->type = TOK_DELIMITER; }
static void identify_operator(Lexer *lexer, Token *token)  { token->type = TOK_LOGIC_OP; }
static void identify_comment(Lexer *lexer, Token *token)   { token->type = TOK_COMMENT; }
static void identify_error(Lexer *lexer, Token *token)     { token->type = TOK_ERROR; }

static void skip_line_comment(Lexer *lexer) {
    lexer->position += 2;
    lexer->column += 2;
    while (lexer->position < lexer->length && lexer->source[lexer->position] != '\n') {
        lexer->position++;
        lexer->column++;
    }
}

static void skip_block_comment(Lexer *lexer) {
    lexer->position += 2;
    lexer->column += 2;
    while (lexer->position < lexer->length) {
        if (lexer->source[lexer->position] == '*' && lexer->source[lexer->position + 1] == '/') {
            lexer->position += 2;
            lexer->column += 2;
            break;
        }
        if (lexer->source[lexer->position] == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        lexer->position++;
    }
}

static void init_token(Token *token) {
    token->type = TOK_ERROR;
    token->lexeme = NULL;
    token->length = 0;
    token->line = 0;
    token->column = 0;
}

void tokenize(Lexer *lexer, TokenList *list) {
    Token token; // Declarar una sola variable token reutilizable

    while (lexer->position < lexer->length) {
        char current = lexer->source[lexer->position];
        init_token(&token); // Inicializar antes de usar

        if (current == '/' && lexer->source[lexer->position + 1] == '/') {
            skip_line_comment(lexer);
            continue;
        }

        if (current == '/' && lexer->source[lexer->position + 1] == '*') {
            skip_block_comment(lexer);
            continue;
        }

        if ((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z') || current == '_') {
            identify_identifier(lexer, &token);
        } else if (current >= '0' && current <= '9') {
            identify_number(lexer, &token);
        } else if (current == '"') {
            identify_string(lexer, &token);
        } else {
            identify_delimiter(lexer, &token);
        }

        add_token(list, token);
        // Actualización de posición, línea y columna
        lexer->position++;
        if (current == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
    }
}