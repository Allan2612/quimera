#include "diag.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Implementación del nuevo sistema de diagnósticos para comunicación IDE-Compilador
 * 
 * Formato de mensajes de error por stderr:
 * {"type":"diagnostic","stage":"lexer","severity":"error","code":1001,"line":1,"column":5,"length":1,"lexeme":"@","message":"Carácter no válido"}
 * 
 * Formato de salida del programa por stdout:
 * {"type":"output","content":"Hello World","output_type":"text"}
 * {"type":"output","content":"42","output_type":"integer"}
 * {"type":"runtime_error","message":"División por cero"}
 */

// Tabla de códigos de error y mensajes predefinidos
typedef struct {
    int code;
    const char *message_template;
} ErrorCodeDef;

static ErrorCodeDef error_codes[] = {
    // Errores del lexer (1000-1999)
    {1001, "Carácter no válido: '%s'"},
    {1002, "Cadena sin cerrar"},
    {1003, "Comentario de bloque sin cerrar"},
    {1004, "Formato de número inválido: '%s'"},
    {1005, "Literal de carácter inválido"},
    {1006, "Literal de carácter sin cerrar"},
    {1007, "Literal de carácter vacío"},
    {1008, "Secuencia de escape inválida: '%s'"},
    
    // Errores del parser (2000-2999)
    {2001, "Se esperaba ';'"},
    {2002, "Token inesperado: '%s'"},
    {2003, "Se esperaba ')'"},
    {2004, "Se esperaba '}'"},
    {2005, "Se esperaba ']'"},
    {2006, "Se esperaba identificador"},
    {2007, "Se esperaba expresión"},
    {2008, "Se esperaba tipo"},
    {2009, "Se esperaba palabra clave: '%s'"},
    {2010, "Se esperaba delimitador: '%s'"},
    
    // Errores semánticos (3000-3999)
    {3001, "Identificador no declarado: '%s'"},
    {3002, "Variable ya declarada: '%s'"},
    {3003, "Tipos incompatibles"},
    {3004, "No se puede asignar a esta expresión"},
    {3005, "Función no definida: '%s'"},
    {3006, "Número incorrecto de argumentos"},
    
    // Errores de runtime (4000-4999)
    {4001, "División por cero"},
    {4002, "Índice fuera de rango"},
    {4003, "Operación no válida entre tipos"},
    {4004, "Stack overflow"},
    {4005, "Valor nulo accedido"},
    
    // Errores del sistema (5000-5999)
    {5001, "Error al abrir archivo: '%s'"},
    {5002, "Error de memoria"},
    {5003, "Archivo no encontrado: '%s'"},
    {5004, "Error de E/O"},
};

static const char* stage_to_string(DiagStage stage) {
    switch (stage) {
        case STAGE_LEXER: return "lexer";
        case STAGE_PARSER: return "parser"; 
        case STAGE_SEMANTIC: return "semantic";
        case STAGE_RUNTIME: return "runtime";
        case STAGE_SYSTEM: return "system";
        default: return "unknown";
    }
}

static const char* severity_to_string(DiagSeverity severity) {
    switch (severity) {
        case SEVERITY_ERROR: return "error";
        case SEVERITY_WARNING: return "warning";
        case SEVERITY_INFO: return "info";
        default: return "unknown";
    }
}

static const char* lookup_error_message(int code) {
    for (size_t i = 0; i < sizeof(error_codes) / sizeof(error_codes[0]); i++) {
        if (error_codes[i].code == code) {
            return error_codes[i].message_template;
        }
    }
    return "Error desconocido (código %d)";
}

static void json_escape_string(const char *src, char *dest, size_t dest_size) {
    if (!src || !dest || dest_size == 0) return;
    
    size_t dest_pos = 0;
    for (size_t i = 0; src[i] && dest_pos < dest_size - 1; i++) {
        switch (src[i]) {
            case '"':
                if (dest_pos < dest_size - 2) {
                    dest[dest_pos++] = '\\';
                    dest[dest_pos++] = '"';
                }
                break;
            case '\\':
                if (dest_pos < dest_size - 2) {
                    dest[dest_pos++] = '\\';
                    dest[dest_pos++] = '\\';
                }
                break;
            case '\n':
                if (dest_pos < dest_size - 2) {
                    dest[dest_pos++] = '\\';
                    dest[dest_pos++] = 'n';
                }
                break;
            case '\r':
                if (dest_pos < dest_size - 2) {
                    dest[dest_pos++] = '\\';
                    dest[dest_pos++] = 'r';
                }
                break;
            case '\t':
                if (dest_pos < dest_size - 2) {
                    dest[dest_pos++] = '\\';
                    dest[dest_pos++] = 't';
                }
                break;
            default:
                dest[dest_pos++] = src[i];
                break;
        }
    }
    dest[dest_pos] = '\0';
}

// ===== Implementación de la API pública =====

void diag_init(void) {
    // Deshabilitar buffering en stdout y stderr para comunicación en tiempo real
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

void diag_shutdown(void) {
    // Forzar flush de cualquier buffer pendiente
    fflush(stdout);
    fflush(stderr);
}

static void report_diagnostic(DiagStage stage, DiagSeverity severity, int code, 
                             DiagLocation location, const char *message) {
    const char *template_msg = lookup_error_message(code);
    
    // Preparar mensaje final
    char final_message[1024];
    if (message) {
        // Usar mensaje personalizado
        strncpy(final_message, message, sizeof(final_message) - 1);
        final_message[sizeof(final_message) - 1] = '\0';
    } else if (location.lexeme && strstr(template_msg, "%s")) {
        // Usar template con lexeme
        snprintf(final_message, sizeof(final_message), template_msg, location.lexeme);
    } else {
        // Usar template sin sustituciones
        strncpy(final_message, template_msg, sizeof(final_message) - 1);
        final_message[sizeof(final_message) - 1] = '\0';
    }
    
    // Escapar strings para JSON
    char escaped_message[2048];
    char escaped_lexeme[512] = "";
    
    json_escape_string(final_message, escaped_message, sizeof(escaped_message));
    if (location.lexeme) {
        json_escape_string(location.lexeme, escaped_lexeme, sizeof(escaped_lexeme));
    }
    
    // Emitir JSON por stderr
    fprintf(stderr, 
        "{\"type\":\"diagnostic\",\"stage\":\"%s\",\"severity\":\"%s\",\"code\":%d,"
        "\"line\":%zu,\"column\":%zu,\"length\":%zu,\"lexeme\":\"%s\",\"message\":\"%s\"}\n",
        stage_to_string(stage),
        severity_to_string(severity),
        code,
        location.line,
        location.column,
        location.length,
        escaped_lexeme,
        escaped_message
    );
}

void diag_error(DiagStage stage, int code, DiagLocation location, const char *message) {
    report_diagnostic(stage, SEVERITY_ERROR, code, location, message);
}

void diag_warning(DiagStage stage, int code, DiagLocation location, const char *message) {
    report_diagnostic(stage, SEVERITY_WARNING, code, location, message);
}

void diag_info(DiagStage stage, int code, DiagLocation location, const char *message) {
    report_diagnostic(stage, SEVERITY_INFO, code, location, message);
}

void diag_error_simple(DiagStage stage, int code, const char *message) {
    DiagLocation loc = {.line = 0, .column = 0, .length = 0, .lexeme = NULL};
    diag_error(stage, code, loc, message);
}

void diag_error_file(const char *filename, const char *operation) {
    char message[512];
    snprintf(message, sizeof(message), "Error al %s el archivo: %s", 
             operation ? operation : "acceder", filename ? filename : "desconocido");
    diag_error_simple(STAGE_SYSTEM, 5001, message);
}

void diag_error_memory(void) {
    diag_error_simple(STAGE_SYSTEM, 5002, "Error de memoria insuficiente");
}

// ===== Salida del programa interpretado =====

void output_program_value(const char *value, const char *type) {
    char escaped_value[1024];
    char escaped_type[64];
    
    json_escape_string(value ? value : "", escaped_value, sizeof(escaped_value));
    json_escape_string(type ? type : "unknown", escaped_type, sizeof(escaped_type));
    
    printf("{\"type\":\"output\",\"content\": \"%s\",\"output_type\":\"%s\"}\n",
           escaped_value, escaped_type);
}

void output_program_text(const char *text) {
    output_program_value(text ? text : "", "text");
}

void output_program_error(const char *error_msg) {
    char escaped_msg[1024];
    json_escape_string(error_msg ? error_msg : "", escaped_msg, sizeof(escaped_msg));
    
    printf("{\"type\":\"runtime_error\",\"message\":\"%s\"}\n", escaped_msg);
}

// ===== Utilidades para DiagLocation =====

DiagLocation diag_location_create(size_t line, size_t column, size_t length, const char *lexeme) {
    DiagLocation loc = {
        .code = 0,  // Se establece en las funciones de reporte
        .line = line,
        .column = column,
        .length = length,
        .lexeme = lexeme
    };
    return loc;
}

DiagLocation diag_location_simple(size_t line, size_t column) {
    return diag_location_create(line, column, 0, NULL);
}