#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOKEN_LIST_INITIAL_CAPACITY 16

// Función para manejar errores
void handleError(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(EXIT_FAILURE);
}

// Función para duplicar cadenas
char *duplicateString(const char *str) {
    char *dup = malloc(strlen(str) + 1);
    if (dup == NULL) {
        handleError("No se pudo asignar memoria para la cadena duplicada.");
    }
    strcpy(dup, str);
    return dup;
}

// Función para imprimir un mensaje de depuración
void debugPrint(const char *message) {
    #ifdef DEBUG
    printf("DEBUG: %s\n", message);
    #endif
}

char *substr_dup(const char *src, size_t start, size_t len) {
    char *s = malloc(len + 1);
    if (!s) return NULL;
    memcpy(s, src + start, len);
    s[len] = '\0';
    return s;
}

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