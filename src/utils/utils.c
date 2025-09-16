#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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