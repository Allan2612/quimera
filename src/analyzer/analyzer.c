#include "analyzer.h"
#include "diag.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* ========== Constantes ========== */

#define SYMBOL_TABLE_INITIAL_SIZE 32
#define SYMBOL_TABLE_MAX_LOAD_FACTOR 0.75

/* ========== Hash table para símbolos ========== */

static size_t hash_string(const char *str) {
    size_t hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + (unsigned char)*str++;
    }
    return hash;
}

static Symbol *symbol_table_find_slot(Symbol **table, size_t capacity, const char *name) {
    size_t index = hash_string(name) % capacity;
    while (table[index] && strcmp(table[index]->name, name) != 0) {
        index = (index + 1) % capacity;
    }
    return table[index];
}

static void symbol_table_insert(Symbol **table, size_t capacity, Symbol *symbol) {
    size_t index = hash_string(symbol->name) % capacity;
    while (table[index]) {
        index = (index + 1) % capacity;
    }
    table[index] = symbol;
}

/* ========== Gestión de ámbitos ========== */

static AnalyzerScope *scope_create(SemanticAnalyzer *analyzer, const char *name, AnalyzerScope *parent) {
    AnalyzerScope *scope = (AnalyzerScope*)calloc(1, sizeof(AnalyzerScope));
    if (!scope) {
        semantic_error(analyzer, (SrcSpan){0}, "Error de memoria al crear ámbito");
        return NULL;
    }
    
    scope->symbols = (Symbol**)calloc(SYMBOL_TABLE_INITIAL_SIZE, sizeof(Symbol*));
    if (!scope->symbols) {
        free(scope);
        semantic_error(analyzer, (SrcSpan){0}, "Error de memoria al crear tabla de símbolos");
        return NULL;
    }
    
    scope->capacity = SYMBOL_TABLE_INITIAL_SIZE;
    scope->count = 0;
    scope->parent = parent;
    scope->name = name;
    scope->is_function_scope = false;
    scope->has_return = false;
    
    return scope;
}

static void scope_destroy(AnalyzerScope *scope) {
    if (!scope) return;
    
    // Liberar símbolos
    for (size_t i = 0; i < scope->capacity; i++) {
        if (scope->symbols[i]) {
            free(scope->symbols[i]);
        }
    }
    free(scope->symbols);
    free(scope);
}

AnalyzerScope *scope_push(SemanticAnalyzer *analyzer, const char *name) {
    AnalyzerScope *new_scope = scope_create(analyzer, name, analyzer->current_scope);
    if (new_scope) {
        analyzer->current_scope = new_scope;
    }
    return new_scope;
}

void scope_pop(SemanticAnalyzer *analyzer) {
    if (!analyzer->current_scope || analyzer->current_scope == analyzer->global_scope) {
        return; // No se puede salir del ámbito global
    }
    
    AnalyzerScope *old_scope = analyzer->current_scope;
    analyzer->current_scope = old_scope->parent;
    scope_destroy(old_scope);
}

/* ========== Gestión de tipos ========== */

TypeInfo type_create_basic(SemanticType type, const char *name) {
    TypeInfo info = {0};
    info.base_type = type;
    info.type_name = name;
    info.element_type = TYPE_UNKNOWN;
    info.array_size = 0;
    info.is_mutable = true;
    return info;
}

TypeInfo type_create_array(SemanticType element_type, size_t size) {
    TypeInfo info = {0};
    info.base_type = TYPE_ARRAY;
    info.element_type = element_type;
    info.array_size = size;
    info.is_mutable = true;
    return info;
}

TypeInfo type_create_list(SemanticType element_type) {
    TypeInfo info = {0};
    info.base_type = TYPE_LIST;
    info.element_type = element_type;
    info.array_size = 0;
    info.is_mutable = true;
    return info;
}

bool types_compatible(TypeInfo a, TypeInfo b) {
    // Tipos exactamente iguales
    if (a.base_type == b.base_type && a.element_type == b.element_type) {
        return true;
    }
    
    // Conversión Integer -> Float
    if (a.base_type == TYPE_INTEGER && b.base_type == TYPE_FLOAT) {
        return true;
    }
    
    // Conversión Char -> String
    if (a.base_type == TYPE_CHAR && b.base_type == TYPE_STRING) {
        return true;
    }
    
    return false;
}

bool types_can_convert(TypeInfo from, TypeInfo to) {
    return types_compatible(from, to);
}

TypeInfo type_from_name(SemanticAnalyzer *analyzer, const char *type_name) {
    if (!type_name) {
        return type_create_basic(TYPE_UNKNOWN, "unknown");
    }
    
    // Verificar si es un tipo compuesto (Array: o List:)
    if (strncmp(type_name, "Array:", 6) == 0) {
        const char *element_type_name = type_name + 6; // saltar "Array:"
        TypeInfo element_type = type_from_name(analyzer, element_type_name);
        return type_create_array(element_type.base_type, 0); // tamaño se determina más tarde
    } else if (strncmp(type_name, "List:", 5) == 0) {
        const char *element_type_name = type_name + 5; // saltar "List:"
        TypeInfo element_type = type_from_name(analyzer, element_type_name);
        return type_create_list(element_type.base_type);
    }
    
    // Comparar con símbolos internados para eficiencia
    if (type_name == analyzer->symbols->T_Integer) {
        return type_create_basic(TYPE_INTEGER, type_name);
    } else if (type_name == analyzer->symbols->T_Float) {
        return type_create_basic(TYPE_FLOAT, type_name);
    } else if (type_name == analyzer->symbols->T_String) {
        return type_create_basic(TYPE_STRING, type_name);
    } else if (type_name == analyzer->symbols->T_Char) {
        return type_create_basic(TYPE_CHAR, type_name);
    } else if (type_name == analyzer->symbols->T_Boolean) {
        return type_create_basic(TYPE_BOOLEAN, type_name);
    } else if (type_name == analyzer->symbols->T_List) {
        return type_create_basic(TYPE_LIST, type_name);
    }
    
    // Fallback: comparación por string
    if (strcmp(type_name, "Integer") == 0) {
        return type_create_basic(TYPE_INTEGER, type_name);
    } else if (strcmp(type_name, "Float") == 0) {
        return type_create_basic(TYPE_FLOAT, type_name);
    } else if (strcmp(type_name, "String") == 0) {
        return type_create_basic(TYPE_STRING, type_name);
    } else if (strcmp(type_name, "Char") == 0) {
        return type_create_basic(TYPE_CHAR, type_name);
    } else if (strcmp(type_name, "Boolean") == 0) {
        return type_create_basic(TYPE_BOOLEAN, type_name);
    } else if (strcmp(type_name, "List") == 0) {
        return type_create_basic(TYPE_LIST, type_name);
    } else if (strcmp(type_name, "Null") == 0) {
        return type_create_basic(TYPE_NULL, type_name);
    }
    
    return type_create_basic(TYPE_UNKNOWN, type_name);
}

const char *type_to_string(TypeInfo type) {
    switch (type.base_type) {
        case TYPE_INTEGER: return "Integer";
        case TYPE_FLOAT: return "Float";
        case TYPE_STRING: return "String";
        case TYPE_CHAR: return "Char";
        case TYPE_BOOLEAN: return "Boolean";
        case TYPE_NULL: return "Null";
        case TYPE_ARRAY: return "Array";
        case TYPE_LIST: return "List";
        case TYPE_FUNCTION: return "Function";
        case TYPE_VOID: return "Void";
        default: return "Unknown";
    }
}

TypeInfo get_literal_type(AstLiteral *literal) {
    switch (literal->tag) {
        case LIT_INT:
            return type_create_basic(TYPE_INTEGER, "Integer");
        case LIT_FLOAT:
            return type_create_basic(TYPE_FLOAT, "Float");
        case LIT_STRING:
            return type_create_basic(TYPE_STRING, "String");
        case LIT_BOOL:
            return type_create_basic(TYPE_BOOLEAN, "Boolean");
        case LIT_NULL:
            return type_create_basic(TYPE_NULL, "Null");
        default:
            return type_create_basic(TYPE_UNKNOWN, "Unknown");
    }
}

/* ========== Gestión de símbolos ========== */

bool symbol_declare(SemanticAnalyzer *analyzer, const char *name, TypeInfo type, SrcSpan location) {
    if (!analyzer->current_scope) {
        semantic_error(analyzer, location, "Error interno: no hay ámbito actual");
        return false;
    }
    
    // Verificar redeclaración en el mismo ámbito
    Symbol *existing = symbol_lookup_current_scope(analyzer, name);
    if (existing) {
        semantic_error(analyzer, location, "Variable '%s' ya declarada en línea %d", 
                      name, existing->declared_at.line_start);
        return false;
    }
    
    // Verificar si es palabra reservada
    if (is_reserved_word(analyzer, name)) {
        semantic_error(analyzer, location, "'%s' es una palabra reservada", name);
        return false;
    }
    
    // Crear nuevo símbolo
    Symbol *symbol = (Symbol*)calloc(1, sizeof(Symbol));
    if (!symbol) {
        semantic_error(analyzer, location, "Error de memoria al declarar símbolo");
        return false;
    }
    
    symbol->name = name;
    symbol->type = type;
    symbol->declared_at = location;
    symbol->is_initialized = false;
    symbol->is_function = (type.base_type == TYPE_FUNCTION);
    
    // Insertar en tabla hash
    AnalyzerScope *scope = analyzer->current_scope;
    
    // Verificar si necesitamos redimensionar
    if ((scope->count + 1) > (scope->capacity * SYMBOL_TABLE_MAX_LOAD_FACTOR)) {
        // Redimensionar tabla
        size_t new_capacity = scope->capacity * 2;
        Symbol **new_table = (Symbol**)calloc(new_capacity, sizeof(Symbol*));
        if (!new_table) {
            free(symbol);
            semantic_error(analyzer, location, "Error de memoria al expandir tabla de símbolos");
            return false;
        }
        
        // Rehacer hash
        for (size_t i = 0; i < scope->capacity; i++) {
            if (scope->symbols[i]) {
                symbol_table_insert(new_table, new_capacity, scope->symbols[i]);
            }
        }
        
        free(scope->symbols);
        scope->symbols = new_table;
        scope->capacity = new_capacity;
    }
    
    symbol_table_insert(scope->symbols, scope->capacity, symbol);
    scope->count++;
    
    return true;
}

Symbol *symbol_lookup(SemanticAnalyzer *analyzer, const char *name) {
    AnalyzerScope *scope = analyzer->current_scope;
    while (scope) {
        Symbol *symbol = symbol_table_find_slot(scope->symbols, scope->capacity, name);
        if (symbol) {
            return symbol;
        }
        scope = scope->parent;
    }
    return NULL;
}

Symbol *symbol_lookup_current_scope(SemanticAnalyzer *analyzer, const char *name) {
    if (!analyzer->current_scope) {
        return NULL;
    }
    return symbol_table_find_slot(analyzer->current_scope->symbols, 
                                 analyzer->current_scope->capacity, name);
}

/* ========== Manejo de errores ========== */

void semantic_error(SemanticAnalyzer *analyzer, SrcSpan location, const char *message, ...) {
    va_list args;
    va_start(args, message);
    
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), message, args);
    buffer[sizeof(buffer) - 1] = '\0';
    
    DiagLocation diag_loc = diag_location_create(location.line_start, location.col_start, 
                                               location.col_end - location.col_start, NULL);
    diag_error(STAGE_SEMANTIC, 3000, diag_loc, buffer);
    
    analyzer->error_count++;
    va_end(args);
}

void semantic_warning(SemanticAnalyzer *analyzer, SrcSpan location, const char *message, ...) {
    va_list args;
    va_start(args, message);
    
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), message, args);
    buffer[sizeof(buffer) - 1] = '\0';
    
    DiagLocation diag_loc = diag_location_create(location.line_start, location.col_start, 
                                               location.col_end - location.col_start, NULL);
    diag_warning(STAGE_SEMANTIC, 3000, diag_loc, buffer);
    
    analyzer->warning_count++;
    va_end(args);
}

/* ========== Utilidades ========== */

bool is_reserved_word(SemanticAnalyzer *analyzer, const char *name) {
    return (name == analyzer->symbols->K_If ||
            name == analyzer->symbols->K_ElseIf ||
            name == analyzer->symbols->K_Else ||
            name == analyzer->symbols->K_For ||
            name == analyzer->symbols->K_While ||
            name == analyzer->symbols->K_Return ||
            name == analyzer->symbols->K_Print ||
            name == analyzer->symbols->K_Read ||
            name == analyzer->symbols->K_And ||
            name == analyzer->symbols->K_Or ||
            name == analyzer->symbols->K_Not ||
            name == analyzer->symbols->K_True ||
            name == analyzer->symbols->K_False ||
            name == analyzer->symbols->K_Null ||
            name == analyzer->symbols->K_End);
}

/* ========== Inicialización y destrucción ========== */

void analyzer_init(SemanticAnalyzer *analyzer, AstArena *arena, Interner *interner, Interned *symbols) {
    memset(analyzer, 0, sizeof(SemanticAnalyzer));
    
    analyzer->arena = arena;
    analyzer->interner = interner;
    analyzer->symbols = symbols;
    analyzer->error_count = 0;
    analyzer->warning_count = 0;
    analyzer->in_function = false;
    
    // Crear ámbito global
    analyzer->global_scope = scope_create(analyzer, "global", NULL);
    analyzer->current_scope = analyzer->global_scope;
}

void analyzer_destroy(SemanticAnalyzer *analyzer) {
    // Destruir todos los ámbitos
    while (analyzer->current_scope && analyzer->current_scope != analyzer->global_scope) {
        scope_pop(analyzer);
    }
    
    if (analyzer->global_scope) {
        scope_destroy(analyzer->global_scope);
    }
    
    memset(analyzer, 0, sizeof(SemanticAnalyzer));
}

/* ========== Validación de operaciones ========== */

TypeInfo validate_binary_operation(SemanticAnalyzer *analyzer, OpCode op, TypeInfo left, TypeInfo right, SrcSpan location) {
    TypeInfo result = type_create_basic(TYPE_UNKNOWN, "unknown");
    
    switch (op) {
        case OP_ADD:
            // Concatenación: String + String, y Char con String (el Char se convierte)
            if ((left.base_type == TYPE_STRING || left.base_type == TYPE_CHAR) &&
                (right.base_type == TYPE_STRING || right.base_type == TYPE_CHAR) &&
                (left.base_type == TYPE_STRING || right.base_type == TYPE_STRING)) {
                result = type_create_basic(TYPE_STRING, "String");
                break;
            }
            // si no son textos, es suma numérica
            /* fall through */
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
            // Operaciones aritméticas: solo entre números
            if ((left.base_type == TYPE_INTEGER || left.base_type == TYPE_FLOAT) &&
                (right.base_type == TYPE_INTEGER || right.base_type == TYPE_FLOAT)) {
                
                // Si alguno es float, el resultado es float
                if (left.base_type == TYPE_FLOAT || right.base_type == TYPE_FLOAT) {
                    result = type_create_basic(TYPE_FLOAT, "Float");
                } else {
                    result = type_create_basic(TYPE_INTEGER, "Integer");
                }
            } else {
                semantic_error(analyzer, location, "Operación aritmética entre tipos incompatibles: %s y %s", 
                             type_to_string(left), type_to_string(right));
            }
            break;
            
        case OP_EQ:
        case OP_NEQ:
            // Igualdad: tipos compatibles o Char/String
            if (types_compatible(left, right) || 
                (left.base_type == TYPE_CHAR && right.base_type == TYPE_STRING) ||
                (left.base_type == TYPE_STRING && right.base_type == TYPE_CHAR)) {
                result = type_create_basic(TYPE_BOOLEAN, "Boolean");
            } else {
                semantic_error(analyzer, location, "Comparación de igualdad entre tipos incompatibles: %s y %s", 
                             type_to_string(left), type_to_string(right));
            }
            break;
            
        case OP_LT:
        case OP_LTE:
        case OP_GT:
        case OP_GTE:
            // Comparación: solo números o strings
            if ((left.base_type == TYPE_INTEGER || left.base_type == TYPE_FLOAT) &&
                (right.base_type == TYPE_INTEGER || right.base_type == TYPE_FLOAT)) {
                result = type_create_basic(TYPE_BOOLEAN, "Boolean");
            } else if (left.base_type == TYPE_STRING && right.base_type == TYPE_STRING) {
                result = type_create_basic(TYPE_BOOLEAN, "Boolean");
            } else {
                semantic_error(analyzer, location, "Comparación ordinal entre tipos incompatibles: %s y %s", 
                             type_to_string(left), type_to_string(right));
            }
            break;
            
        case OP_AND:
        case OP_OR:
            // Operaciones lógicas: solo entre booleanos
            if (left.base_type == TYPE_BOOLEAN && right.base_type == TYPE_BOOLEAN) {
                result = type_create_basic(TYPE_BOOLEAN, "Boolean");
            } else {
                semantic_error(analyzer, location, "Operación lógica entre tipos no booleanos: %s y %s", 
                             type_to_string(left), type_to_string(right));
            }
            break;
            
        default:
            semantic_error(analyzer, location, "Operación binaria no soportada");
            break;
    }
    
    return result;
}

TypeInfo validate_unary_operation(SemanticAnalyzer *analyzer, OpCode op, TypeInfo operand, SrcSpan location) {
    TypeInfo result = type_create_basic(TYPE_UNKNOWN, "unknown");
    
    switch (op) {
        case OP_NEG:
            // Negación aritmética: solo números
            if (operand.base_type == TYPE_INTEGER) {
                result = type_create_basic(TYPE_INTEGER, "Integer");
            } else if (operand.base_type == TYPE_FLOAT) {
                result = type_create_basic(TYPE_FLOAT, "Float");
            } else {
                semantic_error(analyzer, location, "Negación aritmética aplicada a tipo no numérico: %s", 
                             type_to_string(operand));
            }
            break;
            
        case OP_NOT:
            // Negación lógica: solo booleanos
            if (operand.base_type == TYPE_BOOLEAN) {
                result = type_create_basic(TYPE_BOOLEAN, "Boolean");
            } else {
                semantic_error(analyzer, location, "Negación lógica aplicada a tipo no booleano: %s", 
                             type_to_string(operand));
            }
            break;
            
        default:
            semantic_error(analyzer, location, "Operación unaria no soportada");
            break;
    }
    
    return result;
}

bool validate_assignment(SemanticAnalyzer *analyzer, TypeInfo target, TypeInfo source, SrcSpan location) {
    if (types_compatible(source, target)) {
        return true;
    }
    
    semantic_error(analyzer, location, "No se puede asignar %s a %s", 
                  type_to_string(source), type_to_string(target));
    return false;
}

TypeInfo validate_function_call(SemanticAnalyzer *analyzer, Symbol *function, AstList *args, SrcSpan location) {
    if (!function->is_function) {
        semantic_error(analyzer, location, "'%s' no es una función", function->name);
        return type_create_basic(TYPE_UNKNOWN, "unknown");
    }
    
    // TODO: Implementar validación completa de argumentos
    // Por ahora, validación básica: el tipo de retorno está en element_type
    SemanticType return_type = function->type.element_type;
    
    // Analizar argumentos (para detectar errores en las expresiones)
    for (size_t i = 0; i < args->count; i++) {
        analyze_node(analyzer, args->data[i]);
    }
    
    return type_create_basic(return_type, type_to_string(type_create_basic(return_type, "")));
}

/* ========== Análisis de nodos AST ========== */

static TypeInfo analyze_identifier(SemanticAnalyzer *analyzer, AstNode *node);
static TypeInfo analyze_literal(SemanticAnalyzer *analyzer, AstNode *node);
static TypeInfo analyze_binary_op(SemanticAnalyzer *analyzer, AstNode *node);
static TypeInfo analyze_unary_op(SemanticAnalyzer *analyzer, AstNode *node);
static TypeInfo analyze_assignment(SemanticAnalyzer *analyzer, AstNode *node);
static TypeInfo analyze_call(SemanticAnalyzer *analyzer, AstNode *node);
static TypeInfo analyze_index(SemanticAnalyzer *analyzer, AstNode *node);
static void analyze_var_declaration(SemanticAnalyzer *analyzer, AstNode *node);
static void analyze_function_declaration(SemanticAnalyzer *analyzer, AstNode *node);
static void analyze_if_statement(SemanticAnalyzer *analyzer, AstNode *node);
static void analyze_while_statement(SemanticAnalyzer *analyzer, AstNode *node);
static void analyze_for_statement(SemanticAnalyzer *analyzer, AstNode *node);
static void analyze_return_statement(SemanticAnalyzer *analyzer, AstNode *node);
static void analyze_print_statement(SemanticAnalyzer *analyzer, AstNode *node);
static void analyze_read_statement(SemanticAnalyzer *analyzer, AstNode *node);
static void analyze_block(SemanticAnalyzer *analyzer, AstNode *node);

TypeInfo analyze_node(SemanticAnalyzer *analyzer, AstNode *node) {
    if (!node) {
        return type_create_basic(TYPE_UNKNOWN, "unknown");
    }
    
    switch (node->kind) {
        case AST_IDENT:
            return analyze_identifier(analyzer, node);
        case AST_LITERAL:
            return analyze_literal(analyzer, node);
        case AST_BINOP:
            return analyze_binary_op(analyzer, node);
        case AST_UNOP:
            return analyze_unary_op(analyzer, node);
        case AST_ASSIGN:
            return analyze_assignment(analyzer, node);
        case AST_CALL:
            return analyze_call(analyzer, node);
        case AST_INDEX:
            return analyze_index(analyzer, node);
        case AST_VAR_DECL:
            analyze_var_declaration(analyzer, node);
            return type_create_basic(TYPE_VOID, "Void");
        case AST_FUNC_DECL:
            analyze_function_declaration(analyzer, node);
            return type_create_basic(TYPE_VOID, "Void");
        case AST_IF:
            analyze_if_statement(analyzer, node);
            return type_create_basic(TYPE_VOID, "Void");
        case AST_WHILE:
            analyze_while_statement(analyzer, node);
            return type_create_basic(TYPE_VOID, "Void");
        case AST_FOR:
            analyze_for_statement(analyzer, node);
            return type_create_basic(TYPE_VOID, "Void");
        case AST_RETURN:
            analyze_return_statement(analyzer, node);
            return type_create_basic(TYPE_VOID, "Void");
        case AST_PRINT:
            analyze_print_statement(analyzer, node);
            return type_create_basic(TYPE_VOID, "Void");
        case AST_READ:
            analyze_read_statement(analyzer, node);
            return type_create_basic(TYPE_VOID, "Void");
        case AST_BLOCK:
            analyze_block(analyzer, node);
            return type_create_basic(TYPE_VOID, "Void");
        case AST_EXPR:
            return analyze_node(analyzer, node->as.expr.expr);
        default:
            semantic_error(analyzer, node->span, "Nodo AST no soportado: %d", node->kind);
            return type_create_basic(TYPE_UNKNOWN, "unknown");
    }
}

/* ========== Implementación de análisis específicos ========== */

static TypeInfo analyze_identifier(SemanticAnalyzer *analyzer, AstNode *node) {
    const char *name = node->as.ident.lexeme;
    Symbol *symbol = symbol_lookup(analyzer, name);
    
    if (!symbol) {
        semantic_error(analyzer, node->span, "Identificador no declarado: '%s'", name);
        return type_create_basic(TYPE_UNKNOWN, "unknown");
    }
    
    return symbol->type;
}

static TypeInfo analyze_literal(SemanticAnalyzer *analyzer, AstNode *node) {
    (void)analyzer; // Evitar warning de parámetro no usado
    return get_literal_type(&node->as.literal);
}

static TypeInfo analyze_binary_op(SemanticAnalyzer *analyzer, AstNode *node) {
    TypeInfo left_type = analyze_node(analyzer, node->as.binop.left);
    TypeInfo right_type = analyze_node(analyzer, node->as.binop.right);
    
    return validate_binary_operation(analyzer, node->as.binop.op, left_type, right_type, node->span);
}

static TypeInfo analyze_unary_op(SemanticAnalyzer *analyzer, AstNode *node) {
    TypeInfo operand_type = analyze_node(analyzer, node->as.unop.expr);
    
    return validate_unary_operation(analyzer, node->as.unop.op, operand_type, node->span);
}

static TypeInfo analyze_assignment(SemanticAnalyzer *analyzer, AstNode *node) {
    TypeInfo lhs_type = analyze_node(analyzer, node->as.assign.lhs);
    TypeInfo rhs_type = analyze_node(analyzer, node->as.assign.rhs);
    
    // Verificar que el lado izquierdo sea asignable
    if (node->as.assign.lhs->kind != AST_IDENT && node->as.assign.lhs->kind != AST_INDEX) {
        semantic_error(analyzer, node->span, "No se puede asignar a esta expresión");
        return type_create_basic(TYPE_UNKNOWN, "unknown");
    }
    
    // Para asignaciones compuestas, verificar que sean compatibles con la operación
    if (node->as.assign.op != OP_ASSIGN) {
        OpCode arith_op;
        switch (node->as.assign.op) {
            case OP_ADD_ASSIGN: arith_op = OP_ADD; break;
            case OP_SUB_ASSIGN: arith_op = OP_SUB; break;
            case OP_MUL_ASSIGN: arith_op = OP_MUL; break;
            case OP_DIV_ASSIGN: arith_op = OP_DIV; break;
            default:
                semantic_error(analyzer, node->span, "Operador de asignación no soportado");
                return lhs_type;
        }
        
        TypeInfo op_result = validate_binary_operation(analyzer, arith_op, lhs_type, rhs_type, node->span);
        validate_assignment(analyzer, lhs_type, op_result, node->span);
    } else {
        validate_assignment(analyzer, lhs_type, rhs_type, node->span);
    }
    
    return lhs_type;
}

static TypeInfo analyze_call(SemanticAnalyzer *analyzer, AstNode *node) {
    // Verificar si es un método especial antes de analizar el callee
    if (node->as.call.callee->kind == AST_IDENT) {
        const char *func_name = node->as.call.callee->as.ident.lexeme;
        
        // Manejar métodos especiales de lista
        if (strcmp(func_name, "size") == 0) {
            // y.size() - primer argumento debe ser una lista
            if (node->as.call.args.count >= 1) {
                TypeInfo base_type = analyze_node(analyzer, node->as.call.args.data[0]);
                if (base_type.base_type == TYPE_LIST) {
                    return type_create_basic(TYPE_INTEGER, "Integer");
                } else {
                    semantic_error(analyzer, node->span, "size() solo puede aplicarse a listas");
                    return type_create_basic(TYPE_UNKNOWN, "unknown");
                }
            }
            return type_create_basic(TYPE_UNKNOWN, "unknown");
        } else if (strcmp(func_name, "append") == 0) {
            // lista.append(elemento) - primer arg lista, segundo elemento
            if (node->as.call.args.count >= 2) {
                TypeInfo list_type = analyze_node(analyzer, node->as.call.args.data[0]);
                TypeInfo element_type = analyze_node(analyzer, node->as.call.args.data[1]);
                
                if (list_type.base_type == TYPE_LIST) {
                    TypeInfo expected_element = type_create_basic(list_type.element_type, "");
                    validate_assignment(analyzer, expected_element, element_type, node->span);
                    return type_create_basic(TYPE_VOID, "Void");
                } else {
                    semantic_error(analyzer, node->span, "append() solo puede aplicarse a listas");
                    return type_create_basic(TYPE_UNKNOWN, "unknown");
                }
            }
            return type_create_basic(TYPE_UNKNOWN, "unknown");
        } else if (strcmp(func_name, "remove") == 0) {
            // lista.remove(index) - primer arg lista, segundo índice
            if (node->as.call.args.count >= 2) {
                TypeInfo list_type = analyze_node(analyzer, node->as.call.args.data[0]);
                TypeInfo index_type = analyze_node(analyzer, node->as.call.args.data[1]);
                
                if (list_type.base_type == TYPE_LIST) {
                    if (index_type.base_type != TYPE_INTEGER) {
                        semantic_error(analyzer, node->span, "remove() requiere un índice entero");
                    }
                    return type_create_basic(TYPE_VOID, "Void");
                } else {
                    semantic_error(analyzer, node->span, "remove() solo puede aplicarse a listas");
                    return type_create_basic(TYPE_UNKNOWN, "unknown");
                }
            }
            return type_create_basic(TYPE_UNKNOWN, "unknown");
        }
    }
    
    // Si no es un método especial, analizar normalmente
    TypeInfo callee_type = analyze_node(analyzer, node->as.call.callee);
    
    // Analizar argumentos
    for (size_t i = 0; i < node->as.call.args.count; i++) {
        analyze_node(analyzer, node->as.call.args.data[i]);
    }
    
    if (node->as.call.callee->kind == AST_IDENT) {
        const char *func_name = node->as.call.callee->as.ident.lexeme;
        
        // Función normal
        Symbol *func_symbol = symbol_lookup(analyzer, func_name);
        
        if (func_symbol) {
            return validate_function_call(analyzer, func_symbol, &node->as.call.args, node->span);
        }
    }
    
    return callee_type;
}

static TypeInfo analyze_index(SemanticAnalyzer *analyzer, AstNode *node) {
    TypeInfo base_type = analyze_node(analyzer, node->as.index.base);
    TypeInfo index_type = analyze_node(analyzer, node->as.index.index);
    
    // Verificar que el índice sea entero
    if (index_type.base_type != TYPE_INTEGER) {
        semantic_error(analyzer, node->span, "Índice debe ser de tipo Integer, se encontró %s", 
                      type_to_string(index_type));
    }
    
    // Verificar que la base sea indexable
    if (base_type.base_type == TYPE_ARRAY || base_type.base_type == TYPE_LIST) {
        return type_create_basic(base_type.element_type, type_to_string(base_type));
    } else if (base_type.base_type == TYPE_STRING) {
        return type_create_basic(TYPE_CHAR, "Char");
    } else {
        semantic_error(analyzer, node->span, "No se puede indexar tipo %s", type_to_string(base_type));
        return type_create_basic(TYPE_UNKNOWN, "unknown");
    }
}

static void analyze_var_declaration(SemanticAnalyzer *analyzer, AstNode *node) {
    const char *name = node->as.var_decl.name;
    const char *type_name = node->as.var_decl.type_name;
    
    TypeInfo var_type = type_from_name(analyzer, type_name);
    
    if (var_type.base_type == TYPE_UNKNOWN) {
        semantic_error(analyzer, node->span, "Tipo desconocido: '%s'", type_name);
        return;
    }
    
    // Declarar variable
    if (!symbol_declare(analyzer, name, var_type, node->span)) {
        return; // Error ya reportado
    }
    
    // Analizar inicialización si existe
    if (node->as.var_decl.init) {
        // Manejo especial para arreglos y listas con inicialización
        if (var_type.base_type == TYPE_ARRAY || var_type.base_type == TYPE_LIST) {
            // Para arreglos y listas, el init es un AST_BLOCK con los elementos
            if (node->as.var_decl.init->kind == AST_BLOCK) {
                AstList *elements = &node->as.var_decl.init->as.block.stmts;
                
                // Para arreglos: primer elemento es tamaño, segundo es lista de elementos
                // Para listas: solo lista de elementos
                AstList *actual_elements = elements;
                if (var_type.base_type == TYPE_ARRAY && elements->count >= 2) {
                    actual_elements = &elements->data[1]->as.block.stmts;
                }
                
                // Validar que todos los elementos son del tipo correcto
                for (size_t i = 0; i < actual_elements->count; i++) {
                    TypeInfo element_type = analyze_node(analyzer, actual_elements->data[i]);
                    
                    // Crear tipo esperado para el elemento
                    TypeInfo expected_element_type = type_create_basic(var_type.element_type, "");
                    
                    validate_assignment(analyzer, expected_element_type, element_type, 
                                       actual_elements->data[i]->span);
                }
                
                // La inicialización es válida
            } else {
                TypeInfo init_type = analyze_node(analyzer, node->as.var_decl.init);
                validate_assignment(analyzer, var_type, init_type, node->span);
            }
        } else {
            // Inicialización normal para tipos simples
            TypeInfo init_type = analyze_node(analyzer, node->as.var_decl.init);
            validate_assignment(analyzer, var_type, init_type, node->span);
        }
        
        // Marcar como inicializada
        Symbol *symbol = symbol_lookup_current_scope(analyzer, name);
        if (symbol) {
            symbol->is_initialized = true;
        }
    }
}

static void analyze_function_declaration(SemanticAnalyzer *analyzer, AstNode *node) {
    const char *name = node->as.func_decl.name;
    const char *ret_type_name = node->as.func_decl.ret_type;
    
    TypeInfo ret_type = type_from_name(analyzer, ret_type_name);
    if (ret_type.base_type == TYPE_UNKNOWN && strcmp(ret_type_name, "Null") != 0) {
        semantic_error(analyzer, node->span, "Tipo de retorno desconocido: '%s'", ret_type_name);
        return;
    }
    
    if (strcmp(ret_type_name, "Null") == 0) {
        ret_type = type_create_basic(TYPE_VOID, "Null");
    }
    
    // Declarar función en ámbito actual
    TypeInfo func_type = type_create_basic(TYPE_FUNCTION, name);
    func_type.element_type = ret_type.base_type; // Almacenar tipo de retorno
    
    if (!symbol_declare(analyzer, name, func_type, node->span)) {
        return;
    }
    
    // Crear nuevo ámbito para la función
    char scope_name[256];
    snprintf(scope_name, sizeof(scope_name), "function_%s", name);
    AnalyzerScope *func_scope = scope_push(analyzer, scope_name);
    if (!func_scope) {
        return;
    }
    
    func_scope->is_function_scope = true;
    func_scope->return_type = ret_type;
    analyzer->in_function = true;
    analyzer->current_function_return_type = ret_type;
    
    // Analizar parámetros
    for (size_t i = 0; i < node->as.func_decl.params.count; i++) {
        AstNode *param = node->as.func_decl.params.data[i];
        if (param->kind == AST_PARAM) {
            const char *param_name = param->as.param.name;
            const char *param_type_name = param->as.param.type_name;
            
            TypeInfo param_type = type_from_name(analyzer, param_type_name);
            if (param_type.base_type == TYPE_UNKNOWN) {
                semantic_error(analyzer, param->span, "Tipo de parámetro desconocido: '%s'", param_type_name);
                continue;
            }
            
            symbol_declare(analyzer, param_name, param_type, param->span);
        }
    }
    
    // Analizar cuerpo de la función
    if (node->as.func_decl.body) {
        analyze_node(analyzer, node->as.func_decl.body);
    }
    
    // Verificar que todas las rutas tengan return (si no es void)
    if (ret_type.base_type != TYPE_VOID && !func_scope->has_return) {
        semantic_error(analyzer, node->span, 
                      "Función '%s' debe retornar un valor de tipo %s en todas las rutas", 
                      name, type_to_string(ret_type));
    }
    
    analyzer->in_function = false;
    scope_pop(analyzer);
}

static void analyze_if_statement(SemanticAnalyzer *analyzer, AstNode *node) {
    // Analizar condición
    TypeInfo cond_type = analyze_node(analyzer, node->as.if_stmt.cond);
    if (cond_type.base_type != TYPE_BOOLEAN) {
        semantic_error(analyzer, node->span, 
                      "Condición de If debe ser Boolean, se encontró %s", 
                      type_to_string(cond_type));
    }
    
    // Analizar bloque then
    if (node->as.if_stmt.then_blk) {
        analyze_node(analyzer, node->as.if_stmt.then_blk);
    }
    
    // Analizar elseifs
    for (size_t i = 0; i < node->as.if_stmt.elseifs.count; i++) {
        analyze_node(analyzer, node->as.if_stmt.elseifs.data[i]);
    }
    
    // Analizar else
    if (node->as.if_stmt.else_blk) {
        analyze_node(analyzer, node->as.if_stmt.else_blk);
    }
}

static void analyze_while_statement(SemanticAnalyzer *analyzer, AstNode *node) {
    // Analizar condición
    TypeInfo cond_type = analyze_node(analyzer, node->as.while_stmt.cond);
    if (cond_type.base_type != TYPE_BOOLEAN) {
        semantic_error(analyzer, node->span, 
                      "Condición de While debe ser Boolean, se encontró %s", 
                      type_to_string(cond_type));
    }
    
    // Analizar cuerpo
    if (node->as.while_stmt.body) {
        analyze_node(analyzer, node->as.while_stmt.body);
    }
}

static void analyze_for_statement(SemanticAnalyzer *analyzer, AstNode *node) {
    // Crear ámbito para el for
    AnalyzerScope *for_scope = scope_push(analyzer, "for_scope");
    if (!for_scope) return;
    
    // Analizar inicialización
    if (node->as.for_stmt.init) {
        analyze_node(analyzer, node->as.for_stmt.init);
    }
    
    // Analizar condición
    if (node->as.for_stmt.cond) {
        TypeInfo cond_type = analyze_node(analyzer, node->as.for_stmt.cond);
        if (cond_type.base_type != TYPE_BOOLEAN) {
            semantic_error(analyzer, node->span, 
                          "Condición de For debe ser Boolean, se encontró %s", 
                          type_to_string(cond_type));
        }
    }
    
    // Analizar paso
    if (node->as.for_stmt.step) {
        analyze_node(analyzer, node->as.for_stmt.step);
    }
    
    // Analizar cuerpo
    if (node->as.for_stmt.body) {
        analyze_node(analyzer, node->as.for_stmt.body);
    }
    
    scope_pop(analyzer);
}

static void analyze_return_statement(SemanticAnalyzer *analyzer, AstNode *node) {
    if (!analyzer->in_function) {
        semantic_error(analyzer, node->span, "Return fuera de función");
        return;
    }
    
    // Buscar el scope de función hacia arriba en la jerarquía
    AnalyzerScope *func_scope = analyzer->current_scope;
    while (func_scope && !func_scope->is_function_scope) {
        func_scope = func_scope->parent;
    }
    
    if (func_scope && func_scope->is_function_scope) {
        func_scope->has_return = true;
    }
    
    TypeInfo return_type = analyzer->current_function_return_type;
    
    if (node->as.ret.expr) {
        // Return con valor
        TypeInfo expr_type = analyze_node(analyzer, node->as.ret.expr);
        
        if (return_type.base_type == TYPE_VOID) {
            semantic_error(analyzer, node->span, 
                          "Función con retorno Null no puede retornar valor");
        } else {
            validate_assignment(analyzer, return_type, expr_type, node->span);
        }
    } else {
        // Return sin valor
        if (return_type.base_type != TYPE_VOID) {
            semantic_error(analyzer, node->span, 
                          "Función debe retornar valor de tipo %s", 
                          type_to_string(return_type));
        }
    }
}

static void analyze_print_statement(SemanticAnalyzer *analyzer, AstNode *node) {
    // Según Sintaxis.md: Print ( <expresion> ) ; - solo UNA expresión
    if (node->as.print.exprs.count != 1) {
        semantic_error(analyzer, node->span, "Print requiere exactamente una expresión");
        return;
    }
    
    // Analizar la única expresión
    analyze_node(analyzer, node->as.print.exprs.data[0]);
}

static void analyze_read_statement(SemanticAnalyzer *analyzer, AstNode *node) {
    // Analizar todas las variables objetivo
    for (size_t i = 0; i < node->as.read.targets.count; i++) {
        AstNode *target = node->as.read.targets.data[i];
        
        if (target->kind != AST_IDENT) {
            semantic_error(analyzer, target->span, 
                          "Read solo puede asignar a variables");
            continue;
        }
        
        analyze_node(analyzer, target);
    }
}

static void analyze_block(SemanticAnalyzer *analyzer, AstNode *node) {
    // Crear nuevo ámbito para el bloque
    AnalyzerScope *block_scope = scope_push(analyzer, "block");
    if (!block_scope) return;
    
    // Analizar todas las sentencias
    for (size_t i = 0; i < node->as.block.stmts.count; i++) {
        analyze_node(analyzer, node->as.block.stmts.data[i]);
    }
    
    scope_pop(analyzer);
}

bool analyze_program(SemanticAnalyzer *analyzer, AstNode *program) {
    if (!program || program->kind != AST_PROGRAM) {
        semantic_error(analyzer, (SrcSpan){0}, "Nodo de programa inválido");
        return false;
    }
    
    // Analizar todas las declaraciones y sentencias del programa
    for (size_t i = 0; i < program->as.program.decls.count; i++) {
        analyze_node(analyzer, program->as.program.decls.data[i]);
    }
    
    return analyzer->error_count == 0;
}