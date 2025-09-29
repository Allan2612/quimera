#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_LIST_INITIAL_CAPACITY 16

static void resize_token_list(TokenList *list, size_t new_capacity) {
    Token *new_tokens = (Token *)realloc(list->tokens, new_capacity * sizeof(Token));
    if (new_tokens) {
        list->tokens = new_tokens;
        list->capacity = new_capacity;
    }
    // Si realloc falla, la lista original sigue válida, pero se debe manejar el error.
}

void init_token_list(TokenList *list) {
    list->tokens = (Token *)malloc(TOKEN_LIST_INITIAL_CAPACITY * sizeof(Token));
    list->count = 0;
    list->capacity = TOKEN_LIST_INITIAL_CAPACITY;
}

void init_token(Token *token) {
    token->type = TOK_ERROR;
    token->lexeme = NULL;
    token->length = 0;
    token->line = 0;
    token->column = 0;
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

void add_token(TokenList *list, Token token) {
    if (list->count >= list->capacity) {
        resize_token_list(list, list->capacity * 2);
    }
    list->tokens[list->count++] = token;
}
