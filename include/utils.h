#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

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

#endif // UTILS_H