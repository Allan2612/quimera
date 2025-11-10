#include "ast.h"
#include <stdlib.h>
#include <string.h>

#ifndef AST_ARENA_DEFAULT_CAP
#define AST_ARENA_DEFAULT_CAP 4096u
#endif

typedef struct AstArenaBlock {
    unsigned char *mem;
    size_t used, cap;
    struct AstArenaBlock *next;
} AstArenaBlock;

struct AstArena {
    AstArenaBlock *head; /* primer bloque */
    AstArenaBlock *tail; /* bloque actual para bump */
};

static AstArenaBlock *arena_block_new(size_t cap) {
    AstArenaBlock *b = (AstArenaBlock*)calloc(1, sizeof(AstArenaBlock));
    if (!b) return NULL;
    b->mem = (unsigned char*)malloc(cap);
    if (!b->mem) { free(b); return NULL; }
    b->cap = cap;
    b->used = 0;
    b->next = NULL;
    return b;
}

AstArena *ast_arena_new(void) {
    AstArena *a = (AstArena*)calloc(1, sizeof(AstArena));
    if (!a) return NULL;
    AstArenaBlock *b = arena_block_new(AST_ARENA_DEFAULT_CAP);
    if (!b) { free(a); return NULL; }
    a->head = a->tail = b;
    return a;
}

void ast_arena_free(AstArena *a) {
    if (!a) return;
    AstArenaBlock *b = a->head;
    while (b) {
        AstArenaBlock *next = b->next;
        free(b->mem);
        free(b);
        b = next;
    }
    free(a);
}

/* Nunca mueve memoria ya entregada: si no cabe en el bloque actual,
   crea un bloque nuevo y asigna desde ahí. */
static void *arena_alloc(AstArena *a, size_t sz, size_t align) {
    if (align == 0) align = sizeof(void*);
    /* alinear puntero dentro del bloque actual */
    size_t mask = align - 1;
    AstArenaBlock *t = a->tail;
    size_t base = (size_t)(t->used);
    size_t aligned = (base + mask) & ~mask;

    if (aligned + sz <= t->cap) {
        void *p = t->mem + aligned;
        t->used = aligned + sz;
        return p;
    }

    /* Crear bloque nuevo con capacidad suficiente (crece exponencial) */
    size_t need = sz + align; /* margen por alineación */
    size_t newcap = t->cap * 2;
    if (newcap < AST_ARENA_DEFAULT_CAP) newcap = AST_ARENA_DEFAULT_CAP;
    if (newcap < need) newcap = need;

    AstArenaBlock *nb = arena_block_new(newcap);
    if (!nb) return NULL;
    a->tail->next = nb;
    a->tail = nb;

    /* En bloque nuevo, el offset arranca en 0, re-alinear y asignar */
    size_t nbase = 0;
    size_t naligned = (nbase + mask) & ~mask;
    void *p = nb->mem + naligned;
    nb->used = naligned + sz;
    return p;
}

AstNode *ast_new(AstArena *a, AstKind kind, SrcSpan span) {
    AstNode *n = (AstNode*)arena_alloc(a, sizeof(AstNode), _Alignof(AstNode));
    if (!n) return NULL;
    memset(n, 0, sizeof(*n));
    n->kind = kind;
    n->span = span;
    return n;
}

void ast_list_push(AstArena *a, AstList *list, AstNode *node) {
    if (list->count + 1 > list->cap) {
        size_t newcap = list->cap ? list->cap * 2 : 4;
        /* Reservamos NUEVO buffer dentro de la arena (no se mueve nunca). */
        AstNode **buf = (AstNode**)arena_alloc(a, newcap * sizeof(AstNode*), _Alignof(AstNode*));
        if (!buf) return; /* si quieres, puedes agregar assert o manejo de error */
        /* Copiamos el contenido anterior si existía */
        if (list->data && list->count) {
            memcpy(buf, list->data, list->count * sizeof(AstNode*));
        }
        list->data = buf;
        list->cap  = newcap;
    }
    list->data[list->count++] = node;
}

char *ast_arena_strndup(AstArena *a, const char *s, size_t n) {
    if (!s) return NULL;
    char *out = (char*)arena_alloc(a, n + 1, _Alignof(char));  // reserva en la arena
    if (!out) return NULL;
    memcpy(out, s, n);
    out[n] = '\0';
    return out;
}
