#ifndef DIAG_ERRORS_H
#define DIAG_ERRORS_H

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { ST_LEXER=1, ST_PARSER=2, ST_SEMANTIC=3, ST_RUNTIME=4 } Stage;
typedef enum { SV_ERROR=1, SV_WARNING=2 } Severity;

typedef struct {
    int     code;        // p.ej., 1001: char ilegal; 2003: se esperaba ')', etc.
    const char   *lexeme;      // puede ser NULL
    size_t  length;      // opcional (para substr)
    size_t  line;        // 1-based
    size_t  column;      // 1-based
} ErrorToken;

typedef struct {
    Stage    stage;
    Severity severity;
    ErrorToken tok;
    const char *override_msg; // NULL => usar plantilla por code (Opción A)
} MiniDiag;

// Inicializa; si start_worker=true, lanza hilo consumidor que imprime JSONL a sink.
// Si prefieres síncrono (sin hilo) para tests, usa start_worker=false.
void diag_init(bool start_worker, FILE *sink);

// Cierra (une el hilo si existe, limpia cola).
void diag_shutdown(void);

// Reporte principal (Opción A por defecto: code->mensaje). 
// Para Opción B (mensaje libre), pasa override_msg != NULL.
void diag_report(Stage st, Severity sv, ErrorToken tok, const char *override_msg);

// Forzar volcado inmediato de la cola (útil en tests)
void diag_flush(void);
// Versión bloqueante (espera a que la cola quede vacía)
void diag_flush_blocking(void);

#ifdef __cplusplus
}
#endif
#endif
