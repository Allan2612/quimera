#include "diag_errors.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

typedef struct { int code; const char *fmt; } MsgDef;
static MsgDef MSGS[] = {
    // --- Léxico 1xxx ---
    {1001, "Carácter no válido: '%s'"},
    {1002, "Cadena sin cierre."},
    {1003, "Comentario de bloque sin cierre."},
    {1004, "Formato de número inválido: '%s'"},
    {1005, "Literal char inválido."},
    {1006, "No se cerró el literal char."},
    {1007, "Literal char vacío."},
    {1008, "Secuencia de escape inválida en char: '%s'"},
    // --- Sintaxis 2xxx ---
    {2001, "Se esperaba ';'"},
    {2002, "Token inesperado: '%s'"},
    {2003, "Se esperaba ')'"},
    // --- Semántica 3xxx ---
    {3001, "Identificador no declarado: '%s'"},
    {3002, "Tipos incompatibles."},
    // --- Runtime 4xxx ---
    {4001, "índice fuera de rango"},
};

static const char* lookup_msg(int code) {
    for (size_t i=0;i<sizeof(MSGS)/sizeof(MSGS[0]);++i)
        if (MSGS[i].code==code) return MSGS[i].fmt;
    return "error desconocido";
}

// ===== Cola + worker =====
typedef struct Node {
    MiniDiag d;
    char *owned_lexeme;
    struct Node *next;
} Node;

static Node *q_head=NULL, *q_tail=NULL;
static pthread_mutex_t q_mx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  q_cv = PTHREAD_COND_INITIALIZER;
static _Atomic int running = 0;
static pthread_t worker;
static FILE *SINK = NULL;

static const char* stage_str(Stage s){
    switch(s){case ST_LEXER: return "lexer";case ST_PARSER:return "parser";
    case ST_SEMANTIC:return "semantic";default:return "runtime";}
}
static const char* sev_str(Severity v){ return v==SV_ERROR?"error":"warning"; }

static void json_put_escaped(FILE *out, const char *s, size_t n){
    for (size_t i=0;i<n;i++){
        unsigned char c = (unsigned char)s[i];
        switch (c){
            case '\"': fputs("\\\"", out); break;
            case '\\': fputs("\\\\", out); break;
            case '\b': fputs("\\b",  out); break;
            case '\f': fputs("\\f",  out); break;
            case '\n': fputs("\\n",  out); break;
            case '\r': fputs("\\r",  out); break;
            case '\t': fputs("\\t",  out); break;
            default:
                if (c < 0x20) { fprintf(out, "\\u%04x", c); }
                else { fputc(c, out); }
        }
    }
}

// Salida en JSONL compacta (IDE friendly)
static void print_jsonl(const MiniDiag *d){
    const char *fmt = d->override_msg ? d->override_msg : lookup_msg(d->tok.code);

    char buf[1024];
    if (strstr(fmt, "%s"))
        snprintf(buf, sizeof buf, fmt, d->tok.lexeme ? d->tok.lexeme : "");
    else
        snprintf(buf, sizeof buf, "%s", fmt);

    FILE *out = SINK ? SINK : stderr;
    fprintf(out,
        "{\"stage\":\"%s\",\"severity\":\"%s\",\"code\":%d,"
        "\"line\":%zu,\"column\":%zu,\"message\":\"",
        stage_str(d->stage), sev_str(d->severity), d->tok.code,
        d->tok.line, d->tok.column);

    json_put_escaped(out, buf, strlen(buf));
    fputs("\"}\n", out);
    fflush(out);
}

static void* worker_fn(void*_) {
    while (atomic_load(&running)) {
        pthread_mutex_lock(&q_mx);
        while (!q_head && atomic_load(&running))
            pthread_cond_wait(&q_cv, &q_mx);
        Node *n = q_head;
        if (n){ q_head = n->next; if (!q_head) q_tail=NULL; }
        pthread_mutex_unlock(&q_mx);

        if (n){
            print_jsonl(&n->d);
            if (n->owned_lexeme) free(n->owned_lexeme);
            free(n);
        }
    }
    // drena remanente
    for(;;){
        pthread_mutex_lock(&q_mx);
        Node *n = q_head; if (n){ q_head=n->next; if(!q_head) q_tail=NULL; }
        pthread_mutex_unlock(&q_mx);
        if(!n) break;
        print_jsonl(&n->d);
        if (n->owned_lexeme) free(n->owned_lexeme);
        free(n);
    }
    return NULL;
}

void diag_init(bool start_worker, FILE *sink){
    SINK = sink?sink:stderr;
    atomic_store(&running, start_worker?1:0);
    if (start_worker){
        (void)pthread_create(&worker, NULL, worker_fn, NULL);
    }
}

void diag_shutdown(void){
    if (atomic_load(&running)){
        atomic_store(&running, 0);
        pthread_mutex_lock(&q_mx);
        pthread_cond_broadcast(&q_cv);
        pthread_mutex_unlock(&q_mx);
        pthread_join(worker, NULL);
    }
}

static char* dup_lexeme_from_len(const char *src, size_t len){
    // Asumimos 'len' correcto como pediste; normalizamos a cadena NUL-terminada
    if (!src || len == 0){
        char *p = (char*)malloc(1);
        if (p) p[0] = '\0';
        return p;
    }
    char *p = (char*)malloc(len + 1);
    if (!p) return NULL;
    memcpy(p, src, len);
    p[len] = '\0';
    return p;
}

static void enqueue(const MiniDiag *src){
    Node *n = (Node*)malloc(sizeof(Node));
    n->d = *src;
    n->owned_lexeme = NULL;
    n->next = NULL;

    // SIEMPRE copia propia NUL-terminada usando length
    n->owned_lexeme = dup_lexeme_from_len(src->tok.lexeme, src->tok.length);
    n->d.tok.lexeme = n->owned_lexeme;

    pthread_mutex_lock(&q_mx);
    if (q_tail) q_tail->next = n; else q_head = n;
    q_tail = n;
    pthread_cond_signal(&q_cv);
    pthread_mutex_unlock(&q_mx);
}

void diag_report(Stage st, Severity sv, ErrorToken tok, const char *override_msg){
    MiniDiag d = { .stage=st, .severity=sv, .tok=tok, .override_msg=override_msg };
    if (atomic_load(&running)) {
        enqueue(&d);
    } else {
        // Modo síncrono: también normaliza antes de imprimir
        char *owned = dup_lexeme_from_len(d.tok.lexeme, d.tok.length);
        d.tok.lexeme = owned;
        print_jsonl(&d);
        free(owned);
    }
}

void diag_flush(void){
    pthread_mutex_lock(&q_mx);
    pthread_cond_broadcast(&q_cv);
    pthread_mutex_unlock(&q_mx);
}

void diag_flush_blocking(void){
    if (atomic_load(&running)){
        for(;;){
            pthread_mutex_lock(&q_mx);
            bool empty = (q_head == NULL);
            pthread_mutex_unlock(&q_mx);
            if (empty) break;
            struct timespec ts = {0, 10*1000*1000}; // 10 ms
            nanosleep(&ts, NULL);
        }
    }
}
