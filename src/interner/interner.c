#include "interner.h"
#include "ast.h"     // para ast_arena_strndup
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static inline uint64_t fnv1a64(const char *s, size_t n) {
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < n; ++i) {
        h ^= (unsigned char)s[i];
        h *= 1099511628211ULL;
    }
    return h ? h : 1469598103934665603ULL;
}

static inline size_t next_pow2(size_t x) {
    size_t p = 16;
    while (p < x) p <<= 1;
    return p;
}

static void interner_rehash(Interner *I, size_t new_cap) {
    InternEntry *old = I->tab;
    size_t old_cap = I->cap;

    InternEntry *tab = (InternEntry*)calloc(new_cap, sizeof(InternEntry));
    assert(tab);
    I->tab = tab;
    I->cap = new_cap;
    I->count = 0;

    if (!old) return;

    size_t mask = new_cap - 1;
    for (size_t i = 0; i < old_cap; ++i) {
        if (!old[i].s) continue;
        InternEntry e = old[i];
        size_t j = (size_t)(e.h & mask);
        while (tab[j].s) j = (j + 1) & mask;
        tab[j] = e;
        I->count++;
    }
    free(old);
}

void interner_init(Interner *I, struct AstArena *arena, size_t initial_cap) {
    I->arena = arena;
    I->tab   = NULL;
    I->cap   = 0;
    I->count = 0;
    interner_rehash(I, next_pow2(initial_cap ? initial_cap : 64));
}

void interner_destroy(Interner *I) {
    free(I->tab);
    I->tab   = NULL;
    I->cap   = 0;
    I->count = 0;
    I->arena = NULL;
}

static const char *interner_put_new(Interner *I, const char *s, size_t n, uint64_t h) {
    if ((I->count + 1) * 10 >= I->cap * 7) { // load factor ~0.7
        interner_rehash(I, I->cap ? (I->cap << 1) : 64);
    }
    size_t mask = I->cap - 1;
    size_t j = (size_t)(h & mask);

    while (I->tab[j].s) j = (j + 1) & mask;

    char *dup = ast_arena_strndup(I->arena, s, n); // una copia canónica
    InternEntry e = { dup, n, h };
    I->tab[j] = e;
    I->count++;
    return dup;
}

const char *interner_intern_slice(Interner *I, const char *s, size_t n) {
    uint64_t h = fnv1a64(s, n);
    size_t mask = I->cap - 1;
    size_t j = (size_t)(h & mask);

    for (;;) {
        InternEntry *slot = &I->tab[j];
        if (!slot->s) {
            return interner_put_new(I, s, n, h);
        }
        if (slot->h == h && slot->n == n && memcmp(slot->s, s, n) == 0) {
            return slot->s; // ya existía
        }
        j = (j + 1) & mask;
    }
}

const char *interner_intern_cstr(Interner *I, const char *s) {
    return interner_intern_slice(I, s, strlen(s));
}

void interner_bootstrap_quimera(Interner *I, Interned *S) {
    memset(S, 0, sizeof(*S));
    // Tipos
    S->T_Integer = interner_intern_cstr(I, "Integer");
    S->T_Float   = interner_intern_cstr(I, "Float");
    S->T_String  = interner_intern_cstr(I, "String");
    S->T_Boolean = interner_intern_cstr(I, "Boolean");
    S->T_Char    = interner_intern_cstr(I, "Char");
    S->T_List    = interner_intern_cstr(I, "List");

    // Palabras clave
    S->K_If      = interner_intern_cstr(I, "If");
    S->K_ElseIf  = interner_intern_cstr(I, "ElseIf");
    S->K_Else    = interner_intern_cstr(I, "Else");
    S->K_For     = interner_intern_cstr(I, "For");
    S->K_While   = interner_intern_cstr(I, "While");
    S->K_Return  = interner_intern_cstr(I, "Return");
    S->K_Print   = interner_intern_cstr(I, "Print");
    S->K_Read    = interner_intern_cstr(I, "Read");
    S->K_And     = interner_intern_cstr(I, "And");
    S->K_Or      = interner_intern_cstr(I, "Or");
    S->K_Not     = interner_intern_cstr(I, "Not");
    S->K_True    = interner_intern_cstr(I, "True");
    S->K_False   = interner_intern_cstr(I, "False");
    S->K_Null    = interner_intern_cstr(I, "Null");
    S->K_End     = interner_intern_cstr(I, "End");

    // Delimitadores útiles
    S->D_LParen   = interner_intern_cstr(I, "(");
    S->D_RParen   = interner_intern_cstr(I, ")");
    S->D_LBracket = interner_intern_cstr(I, "[");
    S->D_RBracket = interner_intern_cstr(I, "]");
    S->D_Semi     = interner_intern_cstr(I, ";");
    S->D_Comma    = interner_intern_cstr(I, ",");
    S->D_Assign   = interner_intern_cstr(I, "=");
}
