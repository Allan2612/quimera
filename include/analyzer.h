#ifndef ANALYZER_H
#define ANALYZER_H

#include "ast.h"
#include "diag.h"
#include "interner.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Analizador Semántico de Quimera
 * 
 * Implementa todas las reglas semánticas del lenguaje:
 * - Verificación de tipos
 * - Declaración y uso de identificadores
 * - Alcance de variables y funciones
 * - Conversiones implícitas permitidas
 * - Coherencia en operaciones
 * - Validación de estructuras de control
 * - Verificación de returns en funciones
 */

/* ========== Tipos de datos del analizador ========== */

// Tipos básicos del lenguaje Quimera
typedef enum {
    TYPE_UNKNOWN = 0,
    TYPE_INTEGER,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_CHAR,
    TYPE_BOOLEAN,
    TYPE_NULL,
    TYPE_ARRAY,     // Para arrays de cualquier tipo
    TYPE_LIST,      // Para listas de cualquier tipo
    TYPE_FUNCTION,  // Para funciones
    TYPE_VOID       // Para funciones que no retornan valor
} SemanticType;

// Información de tipo con detalles adicionales
typedef struct {
    SemanticType base_type;
    SemanticType element_type;  // Para arrays y listas
    const char *type_name;      // Nombre del tipo (internado)
    size_t array_size;          // Para arrays, 0 si no es array
    bool is_mutable;            // Si la variable puede cambiar
} TypeInfo;

// Información de un símbolo en la tabla
typedef struct Symbol {
    const char *name;           // Nombre internado
    TypeInfo type;              // Información de tipo
    SrcSpan declared_at;        // Ubicación de declaración
    bool is_initialized;        // Si la variable ha sido inicializada
    bool is_function;           // Si es una función
    struct Symbol *next;        // Para lista enlazada en hash table
} Symbol;

// Entrada de ámbito (scope)
typedef struct AnalyzerScope {
    Symbol **symbols;           // Hash table de símbolos
    size_t capacity;           // Capacidad de la tabla hash
    size_t count;              // Número de símbolos
    struct AnalyzerScope *parent;      // Ámbito padre
    const char *name;          // Nombre del ámbito (para debug)
    bool is_function_scope;    // Si es el ámbito de una función
    TypeInfo return_type;      // Tipo de retorno esperado (solo para funciones)
    bool has_return;           // Si el ámbito tiene return
} AnalyzerScope;

// Estado del analizador semántico
typedef struct {
    AstArena *arena;           // Arena para gestión de memoria
    Interner *interner;        // Interner para strings
    Interned *symbols;         // Símbolos predefinidos del lenguaje
    AnalyzerScope *current_scope;      // Ámbito actual
    AnalyzerScope *global_scope;       // Ámbito global
    int error_count;           // Contador de errores
    int warning_count;         // Contador de advertencias
    bool in_function;          // Si estamos analizando dentro de una función
    TypeInfo current_function_return_type; // Tipo de retorno de la función actual
} SemanticAnalyzer;

/* ========== API Principal ========== */

/**
 * Inicializa el analizador semántico
 */
void analyzer_init(SemanticAnalyzer *analyzer, AstArena *arena, Interner *interner, Interned *symbols);

/**
 * Destruye el analizador semántico y libera recursos
 */
void analyzer_destroy(SemanticAnalyzer *analyzer);

/**
 * Analiza un programa completo (punto de entrada principal)
 */
bool analyze_program(SemanticAnalyzer *analyzer, AstNode *program);

/**
 * Analiza un nodo individual
 */
TypeInfo analyze_node(SemanticAnalyzer *analyzer, AstNode *node);

/* ========== API de Tabla de Símbolos ========== */

/**
 * Crea un nuevo ámbito hijo del actual
 */
AnalyzerScope *scope_push(SemanticAnalyzer *analyzer, const char *name);

/**
 * Abandona el ámbito actual y vuelve al padre
 */
void scope_pop(SemanticAnalyzer *analyzer);

/**
 * Declara un símbolo en el ámbito actual
 */
bool symbol_declare(SemanticAnalyzer *analyzer, const char *name, TypeInfo type, SrcSpan location);

/**
 * Busca un símbolo por nombre en la cadena de ámbitos
 */
Symbol *symbol_lookup(SemanticAnalyzer *analyzer, const char *name);

/**
 * Busca un símbolo solo en el ámbito actual
 */
Symbol *symbol_lookup_current_scope(SemanticAnalyzer *analyzer, const char *name);

/* ========== API de Tipos ========== */

/**
 * Crea información de tipo para un tipo básico
 */
TypeInfo type_create_basic(SemanticType type, const char *name);

/**
 * Crea información de tipo para un array
 */
TypeInfo type_create_array(SemanticType element_type, size_t size);

/**
 * Crea información de tipo para una lista
 */
TypeInfo type_create_list(SemanticType element_type);

/**
 * Verifica si dos tipos son compatibles
 */
bool types_compatible(TypeInfo a, TypeInfo b);

/**
 * Verifica si se puede convertir implícitamente de 'from' a 'to'
 */
bool types_can_convert(TypeInfo from, TypeInfo to);

/**
 * Obtiene información de tipo desde nombre de tipo
 */
TypeInfo type_from_name(SemanticAnalyzer *analyzer, const char *type_name);

/**
 * Convierte un tipo a su representación en string
 */
const char *type_to_string(TypeInfo type);

/* ========== API de Validación ========== */

/**
 * Valida una operación binaria entre dos tipos
 */
TypeInfo validate_binary_operation(SemanticAnalyzer *analyzer, OpCode op, TypeInfo left, TypeInfo right, SrcSpan location);

/**
 * Valida una operación unaria
 */
TypeInfo validate_unary_operation(SemanticAnalyzer *analyzer, OpCode op, TypeInfo operand, SrcSpan location);

/**
 * Valida una asignación
 */
bool validate_assignment(SemanticAnalyzer *analyzer, TypeInfo target, TypeInfo source, SrcSpan location);

/**
 * Valida una llamada de función
 */
TypeInfo validate_function_call(SemanticAnalyzer *analyzer, Symbol *function, AstList *args, SrcSpan location);

/* ========== API de Errores ========== */

/**
 * Reporta un error semántico
 */
void semantic_error(SemanticAnalyzer *analyzer, SrcSpan location, const char *message, ...);

/**
 * Reporta una advertencia semántica
 */
void semantic_warning(SemanticAnalyzer *analyzer, SrcSpan location, const char *message, ...);

/* ========== Utilidades ========== */

/**
 * Verifica si un identificador es una palabra reservada
 */
bool is_reserved_word(SemanticAnalyzer *analyzer, const char *name);

/**
 * Obtiene el tipo de un literal AST
 */
TypeInfo get_literal_type(AstLiteral *literal);

#ifdef __cplusplus
}
#endif

#endif // ANALYZER_H