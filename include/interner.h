#ifndef INTERNER_H
#define INTERNER_H

#include <stddef.h>
#include <stdint.h>

struct AstArena;

typedef struct {
    const char *s;   // C-string canónica (en la arena)
    size_t      n;   // largo sin NUL
    uint64_t    h;   // hash (FNV-1a)
} InternEntry;

typedef struct {
    InternEntry *tab;   // tabla open-addressing (heap para poder rehash)
    size_t       cap;   // potencia de 2 (>= 16)
    size_t       count; // entradas ocupadas
    struct AstArena *arena; // donde se guardan las C-strings
} Interner;

// API básica
void        interner_init(Interner *I, struct AstArena *arena, size_t initial_cap);
void        interner_destroy(Interner *I); // libera solo la tabla, NO las strings (están en arena)
const char *interner_intern_slice(Interner *I, const char *s, size_t n);
const char *interner_intern_cstr (Interner *I, const char *s);

// (Opcional pero recomendado) Set de punteros precargados de Quimera
typedef struct {
    // Tipos
    const char *T_Integer, *T_Float, *T_String, *T_Boolean, *T_Char, *T_List;
    // Palabras clave
    const char *K_If, *K_ElseIf, *K_Else, *K_For, *K_While, *K_Return,
               *K_Print, *K_Read, *K_And, *K_Or, *K_Not, *K_True, *K_False,
               *K_Null, *K_End;
    // Delimitadores (útil si quieres igualdad por puntero)
    const char *D_LParen, *D_RParen, *D_LBracket, *D_RBracket,
               *D_Semi, *D_Comma, *D_Assign;
} Interned;

void interner_bootstrap_quimera(Interner *I, Interned *S);

#endif // INTERNER_H
