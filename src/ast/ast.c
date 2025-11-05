#include "ast.h"
#include <stdlib.h>
#include <string.h>

struct AstArena {
    unsigned char *mem;
    size_t used, cap;
};

static void arena_grow(AstArena *a, size_t need) {
    size_t newcap = a->cap ? a->cap : 4096;
    while (newcap < a->used + need) newcap *= 2;
    a->mem = (unsigned char*)realloc(a->mem, newcap);
    a->cap = newcap;
}

static void *arena_alloc(AstArena *a, size_t sz) {
    if (a->used + sz > a->cap) arena_grow(a, sz);
    void *p = a->mem + a->used;
    a->used += sz;
    return p;
}

AstArena *ast_arena_new(void) {
    AstArena *a = (AstArena*)calloc(1, sizeof(AstArena));
    return a;
}

void ast_arena_free(AstArena *a) {
    if (!a) return;
    free(a->mem);
    free(a);
}

AstNode *ast_new(AstArena *a, AstKind kind, SrcSpan span) {
    AstNode *n = (AstNode*)arena_alloc(a, sizeof(AstNode));
    memset(n, 0, sizeof(*n));
    n->kind = kind;
    n->span = span;
    return n;
}

void ast_list_push(AstArena *a, AstList *list, AstNode *node) {
    if (list->count + 1 > list->cap) {
        size_t newcap = list->cap ? list->cap * 2 : 4;
        AstNode **buf = (AstNode**)arena_alloc(a, newcap * sizeof(AstNode*));
        if (list->data && list->cap) {
            /* copiar al nuevo buffer dentro del arena */
            memcpy(buf, list->data, list->count * sizeof(AstNode*));
        }
        list->data = buf;
        list->cap  = newcap;
    }
    list->data[list->count++] = node;
}
