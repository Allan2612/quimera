#ifndef DIAG_H
#define DIAG_H

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Nuevo sistema de diagnósticos unificado para comunicación IDE-Compilador
 * 
 * Características:
 * - Sin buffering para comunicación en tiempo real
 * - Errores/warnings por stderr en formato JSON
 * - Salida de programa por stdout en formato JSON
 * - Sin threads internos - salida inmediata
 * - API consistente para todas las etapas del compilador
 */

typedef enum {
    STAGE_LEXER = 1,
    STAGE_PARSER = 2,
    STAGE_SEMANTIC = 3,
    STAGE_RUNTIME = 4,
    STAGE_SYSTEM = 5  // Para errores del sistema (archivos, memoria, etc.)
} DiagStage;

typedef enum {
    SEVERITY_ERROR = 1,
    SEVERITY_WARNING = 2,
    SEVERITY_INFO = 3
} DiagSeverity;

typedef struct {
    int code;               // Código único del error
    const char *lexeme;     // Texto que causó el error (puede ser NULL)
    size_t line;           // Línea 1-based
    size_t column;         // Columna 1-based
    size_t length;         // Longitud del lexeme (para highlighting)
} DiagLocation;

/**
 * Inicializa el sistema de diagnósticos
 * Configura stdout y stderr sin buffering
 */
void diag_init(void);

/**
 * Limpia el sistema de diagnósticos
 * Fuerza flush de cualquier buffer pendiente
 */
void diag_shutdown(void);

/**
 * Reporta un error/warning estructurado
 * Se envía por stderr en formato JSON
 */
void diag_error(DiagStage stage, int code, DiagLocation location, const char *message);
void diag_warning(DiagStage stage, int code, DiagLocation location, const char *message);
void diag_info(DiagStage stage, int code, DiagLocation location, const char *message);

/**
 * Funciones de conveniencia para reportar errores comunes
 */
void diag_error_simple(DiagStage stage, int code, const char *message);
void diag_error_file(const char *filename, const char *operation);
void diag_error_memory(void);

/**
 * Salida del programa interpretado
 * Se envía por stdout en formato JSON para que el IDE pueda capturarla
 */
void output_program_value(const char *value, const char *type);
void output_program_text(const char *text);
void output_program_error(const char *error_msg);

/**
 * Utilidades para crear DiagLocation
 */
DiagLocation diag_location_create(size_t line, size_t column, size_t length, const char *lexeme);
DiagLocation diag_location_simple(size_t line, size_t column);

#ifdef __cplusplus
}
#endif

#endif // DIAG_H