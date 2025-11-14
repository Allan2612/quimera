# Analizador Semántico - Documentación Completa

> **Componente**: Analizador Semántico de Quimera  
> **Archivo principal**: `src/analyzer/analyzer.c` (~950 líneas)  
> **Header**: `include/analyzer.h` (~225 líneas)  
> **Estado**: ✅ **COMPLETAMENTE IMPLEMENTADO**

---

## 📋 Índice

1. [Resumen General](#resumen-general)
2. [Arquitectura del Analizador](#arquitectura-del-analizador)
3. [Sistema de Tipos](#sistema-de-tipos)
4. [Gestión de Ámbitos](#gestión-de-ámbitos)
5. [Validación de Declaraciones](#validación-de-declaraciones)
6. [Análisis de Expresiones](#análisis-de-expresiones)
7. [Verificación de Operaciones](#verificación-de-operaciones)
8. [Análisis de Funciones](#análisis-de-funciones)
9. [Estructuras de Control](#estructuras-de-control)
10. [Conversiones de Tipos](#conversiones-de-tipos)
11. [Manejo de Errores Semánticos](#manejo-de-errores-semánticos)
12. [Ejemplos de Análisis](#ejemplos-de-análisis)

---

## 🎯 Resumen General

El **Analizador Semántico de Quimera** es la tercera fase del compilador, responsable de verificar la coherencia semántica del programa más allá de su estructura sintáctica. Implementa un sistema completo de **verificación de tipos**, **gestión de ámbitos**, y **validación de reglas semánticas** del lenguaje.

### Características Principales
- ✅ **Sistema de tipos completo** con 10 tipos diferentes
- ✅ **Gestión de ámbitos jerárquica** con hash tables eficientes
- ✅ **Validación exhaustiva** de operaciones y asignaciones
- ✅ **Conversiones automáticas** según reglas del lenguaje
- ✅ **Análisis de funciones** con verificación de parámetros y retorno
- ✅ **Detección robusta de errores** semánticos
- ✅ **Manejo de AST dañado** por errores sintácticos
- ✅ **Integración completa** con sistema de diagnósticos

---

## 🏗️ Arquitectura del Analizador

### Estructura Principal

```c
typedef struct {
    AstArena *arena;           // Arena para gestión de memoria
    Interner *interner;        // Sistema de strings internadas
    Interned *symbols;         // Símbolos predefinidos del lenguaje
    AnalyzerScope *current_scope;      // Ámbito actual
    AnalyzerScope *global_scope;       // Ámbito global
    int error_count;           // Contador de errores
    int warning_count;         // Contador de advertencias
    bool in_function;          // Si estamos analizando dentro de una función
    TypeInfo current_function_return_type; // Tipo de retorno actual
} SemanticAnalyzer;
```

### API Principal

```c
// Inicialización y destrucción
void analyzer_init(SemanticAnalyzer *analyzer, AstArena *arena, 
                  Interner *interner, Interned *symbols);
void analyzer_destroy(SemanticAnalyzer *analyzer);

// Punto de entrada principal
bool analyze_program(SemanticAnalyzer *analyzer, AstNode *program);

// Análisis específicos
TypeInfo analyze_expression(SemanticAnalyzer *analyzer, AstNode *expr);
bool analyze_statement(SemanticAnalyzer *analyzer, AstNode *stmt);
bool analyze_declaration(SemanticAnalyzer *analyzer, AstNode *decl);
```

---

## 📊 Sistema de Tipos

### Tipos Básicos Soportados

```c
typedef enum {
    TYPE_UNKNOWN = 0,    // Tipo desconocido (error)
    TYPE_INTEGER,        // Números enteros: 42, 0, -15
    TYPE_FLOAT,          // Números flotantes: 3.14, 0.5
    TYPE_STRING,         // Cadenas de texto: "Hello"
    TYPE_CHAR,           // Caracteres: 'a', 'Z'
    TYPE_BOOLEAN,        // Valores lógicos: True, False
    TYPE_NULL,           // Valor nulo: Null
    TYPE_ARRAY,          // Arreglos homogéneos
    TYPE_LIST,           // Listas dinámicas
    TYPE_FUNCTION,       // Funciones
    TYPE_VOID            // Tipo vacío (solo retorno)
} SemanticType;
```

### Información Extendida de Tipos

```c
typedef struct {
    SemanticType base_type;      // Tipo principal
    SemanticType element_type;   // Tipo de elementos (arrays/listas)
    const char *type_name;       // Nombre internado del tipo
    size_t array_size;           // Tamaño de array (0 = no array)
    bool is_mutable;             // Si puede modificarse
} TypeInfo;
```

### Creación y Comparación de Tipos

```c
// Funciones de utilidad para tipos
TypeInfo create_basic_type(SemanticType type, const char *name);
TypeInfo create_array_type(SemanticType element_type, size_t size);
TypeInfo create_list_type(SemanticType element_type);
TypeInfo create_function_type(SemanticType return_type);

bool types_equal(TypeInfo a, TypeInfo b);
bool types_compatible(TypeInfo a, TypeInfo b);
bool is_numeric_type(TypeInfo type);
bool is_convertible(TypeInfo from, TypeInfo to);

// Ejemplos de uso:
TypeInfo int_type = create_basic_type(TYPE_INTEGER, "Integer");
TypeInfo float_array = create_array_type(TYPE_FLOAT, 10);
TypeInfo string_list = create_list_type(TYPE_STRING);
```

### Mapeo de Nombres a Tipos

```c
static TypeInfo resolve_type_name(SemanticAnalyzer *analyzer, const char *type_name) {
    const char *interned_name = interner_intern_cstr(analyzer->interner, type_name);
    
    if (interned_name == analyzer->symbols->T_Integer) {
        return create_basic_type(TYPE_INTEGER, interned_name);
    } else if (interned_name == analyzer->symbols->T_Float) {
        return create_basic_type(TYPE_FLOAT, interned_name);
    } else if (interned_name == analyzer->symbols->T_String) {
        return create_basic_type(TYPE_STRING, interned_name);
    } else if (interned_name == analyzer->symbols->T_Boolean) {
        return create_basic_type(TYPE_BOOLEAN, interned_name);
    } else if (interned_name == analyzer->symbols->T_Char) {
        return create_basic_type(TYPE_CHAR, interned_name);
    } else if (interned_name == analyzer->symbols->T_List) {
        // Las listas requieren especificación de tipo de elemento
        return create_list_type(TYPE_UNKNOWN); // Se resolverá después
    } else {
        // Tipo no reconocido
        return create_basic_type(TYPE_UNKNOWN, interned_name);
    }
}
```

---

## 🔍 Gestión de Ámbitos

### Estructura de Ámbito

```c
typedef struct AnalyzerScope {
    Symbol **symbols;           // Hash table de símbolos
    size_t capacity;           // Capacidad de la tabla hash
    size_t count;              // Número de símbolos actuales
    struct AnalyzerScope *parent;      // Ámbito padre (jerarquía)
    const char *name;          // Nombre del ámbito (debug)
    bool is_function_scope;    // Si es ámbito de función
    TypeInfo return_type;      // Tipo de retorno esperado
    bool has_return;           // Si tiene sentencia return
} AnalyzerScope;
```

### Hash Table Eficiente

```c
// Función hash para nombres de símbolos
static size_t hash_string(const char *str, size_t capacity) {
    size_t hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + *str;
        str++;
    }
    return hash % capacity;
}

// Inserción en tabla hash con rehashing automático
static bool scope_insert_symbol(AnalyzerScope *scope, Symbol *symbol) {
    // Verificar si necesitamos redimensionar
    if (scope->count >= scope->capacity * 0.75) {
        scope_resize(scope, scope->capacity * 2);
    }
    
    size_t index = hash_string(symbol->name, scope->capacity);
    
    // Verificar duplicados en el mismo ámbito
    Symbol *existing = scope->symbols[index];
    while (existing) {
        if (existing->name == symbol->name) {
            return false; // Símbolo ya existe
        }
        existing = existing->next;
    }
    
    // Insertar al inicio de la lista enlazada
    symbol->next = scope->symbols[index];
    scope->symbols[index] = symbol;
    scope->count++;
    
    return true;
}
```

### Operaciones de Ámbito

```c
// Crear nuevo ámbito
AnalyzerScope *scope_create(SemanticAnalyzer *analyzer, AnalyzerScope *parent, 
                           const char *name);

// Entrar en nuevo ámbito
void analyzer_enter_scope(SemanticAnalyzer *analyzer, const char *name);

// Salir del ámbito actual
void analyzer_exit_scope(SemanticAnalyzer *analyzer);

// Buscar símbolo en jerarquía de ámbitos
Symbol *symbol_lookup(SemanticAnalyzer *analyzer, const char *name) {
    AnalyzerScope *current = analyzer->current_scope;
    
    while (current) {
        size_t index = hash_string(name, current->capacity);
        Symbol *symbol = current->symbols[index];
        
        while (symbol) {
            if (symbol->name == name) {
                return symbol; // Encontrado
            }
            symbol = symbol->next;
        }
        
        current = current->parent; // Buscar en ámbito padre
    }
    
    return NULL; // No encontrado
}

// Declarar nuevo símbolo
bool symbol_declare(SemanticAnalyzer *analyzer, const char *name, 
                   TypeInfo type, SrcSpan location) {
    // Solo verificar duplicados en ámbito actual (no en padres)
    AnalyzerScope *current = analyzer->current_scope;
    size_t index = hash_string(name, current->capacity);
    Symbol *existing = current->symbols[index];
    
    while (existing) {
        if (existing->name == name) {
            // Error: redeclaración en el mismo ámbito
            semantic_error(analyzer, location, 3004, 
                         "Redeclaración de identificador '%s'", name);
            return false;
        }
        existing = existing->next;
    }
    
    // Crear nuevo símbolo
    Symbol *symbol = arena_alloc(analyzer->arena, sizeof(Symbol));
    symbol->name = interner_intern_cstr(analyzer->interner, name);
    symbol->type = type;
    symbol->declared_at = location;
    symbol->is_initialized = false;
    symbol->is_function = (type.base_type == TYPE_FUNCTION);
    symbol->next = NULL;
    
    return scope_insert_symbol(current, symbol);
}
```

### Ejemplo de Jerarquía de Ámbitos

```c
// Código fuente:
Integer global_x = 10;

Integer func(Integer param) {
    Integer local_y = 20;
    
    If param > 0 {
        Integer block_z = 30;
        Return local_y + block_z;
    }
    
    Return global_x;
} End

// Jerarquía de ámbitos resultante:
Global Scope:
├── global_x: Integer
├── func: Function(Integer) → Integer
│
└── Function Scope (func):
    ├── param: Integer
    ├── local_y: Integer
    │
    └── Block Scope (if):
        └── block_z: Integer
```

---

## 📝 Validación de Declaraciones

### Análisis de Declaraciones de Variables

```c
bool analyze_declaration(SemanticAnalyzer *analyzer, AstNode *decl) {
    if (!decl || decl->type != AST_DECLARATION) {
        return false;
    }
    
    // Resolver tipo
    const char *type_name = decl->data.declaration.type_name;
    TypeInfo var_type = resolve_type_name(analyzer, type_name);
    
    if (var_type.base_type == TYPE_UNKNOWN) {
        semantic_error(analyzer, decl->span, 3001,
                      "Tipo de dato no reconocido: '%s'", type_name);
        return false;
    }
    
    // Verificar nombre de variable
    const char *var_name = decl->data.declaration.identifier;
    if (!var_name) {
        semantic_error(analyzer, decl->span, 3002,
                      "Nombre de variable faltante en declaración");
        return false;
    }
    
    // Declarar símbolo
    if (!symbol_declare(analyzer, var_name, var_type, decl->span)) {
        // Error ya reportado por symbol_declare
        return false;
    }
    
    // Analizar inicializador si existe
    if (decl->data.declaration.initializer) {
        TypeInfo init_type = analyze_expression(analyzer, 
                                               decl->data.declaration.initializer);
        
        if (!types_compatible_for_assignment(var_type, init_type)) {
            semantic_error(analyzer, decl->data.declaration.initializer->span, 3005,
                          "Tipo de inicializador incompatible: se esperaba '%s', se obtuvo '%s'",
                          var_type.type_name, get_type_name(init_type));
            return false;
        }
        
        // Marcar como inicializada
        Symbol *symbol = symbol_lookup(analyzer, var_name);
        if (symbol) {
            symbol->is_initialized = true;
        }
    }
    
    return true;
}
```

### Validación de Tipos Complejos

```c
// Declaraciones de arrays
Integer arr[10];    // Array de enteros de tamaño fijo

// Declaraciones de listas
List<Integer> numbers;  // Lista de enteros (sintaxis futura)
List items;            // Lista de cualquier tipo (implementación actual)

static TypeInfo analyze_array_declaration(SemanticAnalyzer *analyzer, 
                                         AstNode *decl) {
    // Analizar tipo de elemento
    TypeInfo element_type = resolve_type_name(analyzer, 
                                            decl->data.declaration.element_type);
    
    // Verificar tamaño del array
    size_t array_size = 0;
    if (decl->data.declaration.size_expression) {
        TypeInfo size_type = analyze_expression(analyzer, 
                                               decl->data.declaration.size_expression);
        
        if (size_type.base_type != TYPE_INTEGER) {
            semantic_error(analyzer, decl->span, 3006,
                          "Tamaño de array debe ser un entero");
            return create_basic_type(TYPE_UNKNOWN, "error");
        }
        
        // Evaluar tamaño si es literal
        if (decl->data.declaration.size_expression->type == AST_INTEGER_LITERAL) {
            array_size = atoi(decl->data.declaration.size_expression->data.literal.value);
        }
    }
    
    return create_array_type(element_type.base_type, array_size);
}
```

---

## 🔍 Análisis de Expresiones

### Análisis de Expresiones Básicas

```c
TypeInfo analyze_expression(SemanticAnalyzer *analyzer, AstNode *expr) {
    if (!expr) {
        return create_basic_type(TYPE_UNKNOWN, "null");
    }
    
    switch (expr->type) {
        case AST_INTEGER_LITERAL:
            return create_basic_type(TYPE_INTEGER, "Integer");
            
        case AST_FLOAT_LITERAL:
            return create_basic_type(TYPE_FLOAT, "Float");
            
        case AST_STRING_LITERAL:
            return create_basic_type(TYPE_STRING, "String");
            
        case AST_CHAR_LITERAL:
            return create_basic_type(TYPE_CHAR, "Char");
            
        case AST_BOOLEAN_LITERAL:
            return create_basic_type(TYPE_BOOLEAN, "Boolean");
            
        case AST_NULL_LITERAL:
            return create_basic_type(TYPE_NULL, "Null");
            
        case AST_IDENTIFIER:
            return analyze_identifier(analyzer, expr);
            
        case AST_BINARY_OP:
            return analyze_binary_operation(analyzer, expr);
            
        case AST_UNARY_OP:
            return analyze_unary_operation(analyzer, expr);
            
        case AST_FUNCTION_CALL:
            return analyze_function_call(analyzer, expr);
            
        case AST_INDEX_ACCESS:
            return analyze_index_access(analyzer, expr);
            
        case AST_MEMBER_ACCESS:
            return analyze_member_access(analyzer, expr);
            
        case AST_ERROR:
            // Nodo de error del parser, continuar análisis
            return create_basic_type(TYPE_UNKNOWN, "error");
            
        default:
            semantic_error(analyzer, expr->span, 3007,
                          "Tipo de expresión no soportado");
            return create_basic_type(TYPE_UNKNOWN, "unsupported");
    }
}
```

### Análisis de Identificadores

```c
static TypeInfo analyze_identifier(SemanticAnalyzer *analyzer, AstNode *expr) {
    const char *name = expr->data.literal.value;
    
    // Buscar símbolo en tabla de símbolos
    Symbol *symbol = symbol_lookup(analyzer, name);
    
    if (!symbol) {
        semantic_error(analyzer, expr->span, 3008,
                      "Identificador no declarado: '%s'", name);
        return create_basic_type(TYPE_UNKNOWN, "undeclared");
    }
    
    // Verificar si está inicializado (solo para variables)
    if (!symbol->is_function && !symbol->is_initialized) {
        semantic_warning(analyzer, expr->span, 3009,
                        "Uso de variable posiblemente no inicializada: '%s'", name);
    }
    
    return symbol->type;
}
```

### Análisis de Llamadas a Función

```c
static TypeInfo analyze_function_call(SemanticAnalyzer *analyzer, AstNode *call) {
    const char *func_name = call->data.function_call.name;
    
    // Buscar función
    Symbol *func_symbol = symbol_lookup(analyzer, func_name);
    
    if (!func_symbol) {
        semantic_error(analyzer, call->span, 3010,
                      "Función no declarada: '%s'", func_name);
        return create_basic_type(TYPE_UNKNOWN, "unknown_function");
    }
    
    if (!func_symbol->is_function) {
        semantic_error(analyzer, call->span, 3011,
                      "'%s' no es una función", func_name);
        return create_basic_type(TYPE_UNKNOWN, "not_function");
    }
    
    // Verificar número de argumentos
    size_t expected_args = get_function_param_count(func_symbol);
    size_t actual_args = call->data.function_call.arg_count;
    
    if (actual_args != expected_args) {
        semantic_error(analyzer, call->span, 3012,
                      "Número incorrecto de argumentos: se esperaban %zu, se obtuvieron %zu",
                      expected_args, actual_args);
        return func_symbol->type; // Retornar tipo de función pero con error
    }
    
    // Verificar tipos de argumentos
    for (size_t i = 0; i < actual_args; i++) {
        TypeInfo arg_type = analyze_expression(analyzer, call->data.function_call.args[i]);
        TypeInfo param_type = get_function_param_type(func_symbol, i);
        
        if (!types_compatible_for_assignment(param_type, arg_type)) {
            semantic_error(analyzer, call->data.function_call.args[i]->span, 3013,
                          "Tipo de argumento %zu incorrecto: se esperaba '%s', se obtuvo '%s'",
                          i + 1, param_type.type_name, get_type_name(arg_type));
        }
    }
    
    // Retornar tipo de retorno de la función
    return get_function_return_type(func_symbol);
}
```

---

## ⚙️ Verificación de Operaciones

### Operaciones Binarias

```c
static TypeInfo analyze_binary_operation(SemanticAnalyzer *analyzer, AstNode *op) {
    const char *operator = op->data.binary_op.operator;
    
    TypeInfo left_type = analyze_expression(analyzer, op->data.binary_op.left);
    TypeInfo right_type = analyze_expression(analyzer, op->data.binary_op.right);
    
    // Operaciones aritméticas
    if (strcmp(operator, "+") == 0 || strcmp(operator, "-") == 0 ||
        strcmp(operator, "*") == 0 || strcmp(operator, "/") == 0 ||
        strcmp(operator, "%") == 0) {
        
        return analyze_arithmetic_operation(analyzer, operator, left_type, right_type, op->span);
    }
    
    // Operaciones de comparación
    if (strcmp(operator, "<") == 0 || strcmp(operator, ">") == 0 ||
        strcmp(operator, "<=") == 0 || strcmp(operator, ">=") == 0) {
        
        return analyze_comparison_operation(analyzer, operator, left_type, right_type, op->span);
    }
    
    // Operaciones de igualdad
    if (strcmp(operator, "==") == 0 || strcmp(operator, "!=") == 0) {
        return analyze_equality_operation(analyzer, operator, left_type, right_type, op->span);
    }
    
    // Operaciones lógicas
    if (strcmp(operator, "&&") == 0 || strcmp(operator, "||") == 0) {
        return analyze_logical_operation(analyzer, operator, left_type, right_type, op->span);
    }
    
    semantic_error(analyzer, op->span, 3014,
                  "Operador binario no reconocido: '%s'", operator);
    return create_basic_type(TYPE_UNKNOWN, "invalid_op");
}
```

### Operaciones Aritméticas

```c
static TypeInfo analyze_arithmetic_operation(SemanticAnalyzer *analyzer,
                                           const char *operator,
                                           TypeInfo left, TypeInfo right,
                                           SrcSpan span) {
    // Verificar que ambos operandos sean numéricos
    if (!is_numeric_type(left)) {
        semantic_error(analyzer, span, 3015,
                      "Operando izquierdo de '%s' debe ser numérico", operator);
        return create_basic_type(TYPE_UNKNOWN, "non_numeric");
    }
    
    if (!is_numeric_type(right)) {
        semantic_error(analyzer, span, 3016,
                      "Operando derecho de '%s' debe ser numérico", operator);
        return create_basic_type(TYPE_UNKNOWN, "non_numeric");
    }
    
    // Reglas de promoción de tipos
    if (left.base_type == TYPE_FLOAT || right.base_type == TYPE_FLOAT) {
        return create_basic_type(TYPE_FLOAT, "Float");
    } else {
        return create_basic_type(TYPE_INTEGER, "Integer");
    }
}
```

### Operaciones Lógicas

```c
static TypeInfo analyze_logical_operation(SemanticAnalyzer *analyzer,
                                        const char *operator,
                                        TypeInfo left, TypeInfo right,
                                        SrcSpan span) {
    // Ambos operandos deben ser Boolean
    if (left.base_type != TYPE_BOOLEAN) {
        semantic_error(analyzer, span, 3017,
                      "Operando izquierdo de '%s' debe ser Boolean", operator);
    }
    
    if (right.base_type != TYPE_BOOLEAN) {
        semantic_error(analyzer, span, 3018,
                      "Operando derecho de '%s' debe ser Boolean", operator);
    }
    
    // El resultado siempre es Boolean
    return create_basic_type(TYPE_BOOLEAN, "Boolean");
}
```

### Operaciones de Comparación

```c
static TypeInfo analyze_comparison_operation(SemanticAnalyzer *analyzer,
                                           const char *operator,
                                           TypeInfo left, TypeInfo right,
                                           SrcSpan span) {
    // Los operandos deben ser del mismo tipo o compatibles
    if (!types_compatible_for_comparison(left, right)) {
        semantic_error(analyzer, span, 3019,
                      "Tipos incompatibles para comparación '%s': '%s' y '%s'",
                      operator, left.type_name, right.type_name);
    }
    
    // Solo tipos numéricos y char pueden ser comparados con <, >, <=, >=
    if (!is_numeric_type(left) && left.base_type != TYPE_CHAR) {
        semantic_error(analyzer, span, 3020,
                      "Tipo no soportado para comparación ordenada: '%s'", left.type_name);
    }
    
    // El resultado siempre es Boolean
    return create_basic_type(TYPE_BOOLEAN, "Boolean");
}
```

---

## 🔧 Análisis de Funciones

### Declaración de Funciones

```c
bool analyze_function_definition(SemanticAnalyzer *analyzer, AstNode *func_def) {
    // Obtener información de la función
    const char *func_name = func_def->data.function_def.name;
    const char *return_type_name = func_def->data.function_def.return_type;
    
    // Resolver tipo de retorno
    TypeInfo return_type = resolve_type_name(analyzer, return_type_name);
    if (return_type.base_type == TYPE_UNKNOWN) {
        semantic_error(analyzer, func_def->span, 3021,
                      "Tipo de retorno no reconocido: '%s'", return_type_name);
        return false;
    }
    
    // Crear tipo de función
    TypeInfo func_type = create_function_type(return_type.base_type);
    
    // Declarar función en ámbito actual
    if (!symbol_declare(analyzer, func_name, func_type, func_def->span)) {
        return false; // Error ya reportado
    }
    
    // Entrar en nuevo ámbito para la función
    analyzer_enter_scope(analyzer, func_name);
    analyzer->in_function = true;
    analyzer->current_function_return_type = return_type;
    analyzer->current_scope->is_function_scope = true;
    analyzer->current_scope->return_type = return_type;
    analyzer->current_scope->has_return = false;
    
    // Analizar parámetros
    for (size_t i = 0; i < func_def->data.function_def.param_count; i++) {
        AstNode *param = func_def->data.function_def.params[i];
        if (!analyze_parameter(analyzer, param)) {
            // Error en parámetro, continuar con otros
        }
    }
    
    // Analizar cuerpo de función
    bool body_ok = analyze_statement(analyzer, func_def->data.function_def.body);
    
    // Verificar que funciones non-void tengan return
    if (return_type.base_type != TYPE_VOID && !analyzer->current_scope->has_return) {
        semantic_error(analyzer, func_def->span, 3022,
                      "Función '%s' debe tener al menos una sentencia Return", func_name);
        body_ok = false;
    }
    
    // Salir del ámbito de función
    analyzer_exit_scope(analyzer);
    analyzer->in_function = false;
    
    return body_ok;
}
```

### Validación de Parámetros

```c
static bool analyze_parameter(SemanticAnalyzer *analyzer, AstNode *param) {
    const char *param_type_name = param->data.parameter.type_name;
    const char *param_name = param->data.parameter.name;
    
    // Resolver tipo del parámetro
    TypeInfo param_type = resolve_type_name(analyzer, param_type_name);
    if (param_type.base_type == TYPE_UNKNOWN) {
        semantic_error(analyzer, param->span, 3023,
                      "Tipo de parámetro no reconocido: '%s'", param_type_name);
        return false;
    }
    
    // Declarar parámetro en ámbito de función
    if (!symbol_declare(analyzer, param_name, param_type, param->span)) {
        return false; // Error ya reportado (parámetro duplicado)
    }
    
    // Marcar parámetro como inicializado (viene del caller)
    Symbol *param_symbol = symbol_lookup(analyzer, param_name);
    if (param_symbol) {
        param_symbol->is_initialized = true;
    }
    
    return true;
}
```

### Análisis de Sentencias Return

```c
static bool analyze_return_statement(SemanticAnalyzer *analyzer, AstNode *ret_stmt) {
    if (!analyzer->in_function) {
        semantic_error(analyzer, ret_stmt->span, 3024,
                      "Sentencia Return fuera de función");
        return false;
    }
    
    TypeInfo expected_type = analyzer->current_function_return_type;
    
    // Marcar que esta función tiene return
    analyzer->current_scope->has_return = true;
    
    // Verificar valor de retorno
    if (ret_stmt->data.return_stmt.expression) {
        // Return con valor
        TypeInfo return_type = analyze_expression(analyzer, ret_stmt->data.return_stmt.expression);
        
        if (expected_type.base_type == TYPE_VOID) {
            semantic_error(analyzer, ret_stmt->span, 3025,
                          "Función void no puede retornar valor");
            return false;
        }
        
        if (!types_compatible_for_assignment(expected_type, return_type)) {
            semantic_error(analyzer, ret_stmt->span, 3026,
                          "Tipo de retorno incorrecto: se esperaba '%s', se obtuvo '%s'",
                          expected_type.type_name, get_type_name(return_type));
            return false;
        }
    } else {
        // Return sin valor
        if (expected_type.base_type != TYPE_VOID) {
            semantic_error(analyzer, ret_stmt->span, 3027,
                          "Función debe retornar un valor de tipo '%s'", expected_type.type_name);
            return false;
        }
    }
    
    return true;
}
```

---

## 🔄 Estructuras de Control

### Análisis de Condicionales If

```c
static bool analyze_if_statement(SemanticAnalyzer *analyzer, AstNode *if_stmt) {
    bool all_ok = true;
    
    // Analizar condición principal
    TypeInfo condition_type = analyze_expression(analyzer, if_stmt->data.if_stmt.condition);
    if (condition_type.base_type != TYPE_BOOLEAN) {
        semantic_error(analyzer, if_stmt->data.if_stmt.condition->span, 3028,
                      "Condición de If debe ser Boolean, se obtuvo '%s'", 
                      get_type_name(condition_type));
        all_ok = false;
    }
    
    // Analizar bloque then
    analyzer_enter_scope(analyzer, "if_then");
    bool then_ok = analyze_statement(analyzer, if_stmt->data.if_stmt.then_block);
    analyzer_exit_scope(analyzer);
    
    if (!then_ok) all_ok = false;
    
    // Analizar bloques ElseIf
    for (size_t i = 0; i < if_stmt->data.if_stmt.elseif_count; i++) {
        TypeInfo elseif_condition_type = analyze_expression(analyzer, 
                                                           if_stmt->data.if_stmt.elseif_conditions[i]);
        
        if (elseif_condition_type.base_type != TYPE_BOOLEAN) {
            semantic_error(analyzer, if_stmt->data.if_stmt.elseif_conditions[i]->span, 3029,
                          "Condición de ElseIf debe ser Boolean");
            all_ok = false;
        }
        
        analyzer_enter_scope(analyzer, "elseif");
        bool elseif_ok = analyze_statement(analyzer, if_stmt->data.if_stmt.elseif_blocks[i]);
        analyzer_exit_scope(analyzer);
        
        if (!elseif_ok) all_ok = false;
    }
    
    // Analizar bloque Else (si existe)
    if (if_stmt->data.if_stmt.else_block) {
        analyzer_enter_scope(analyzer, "else");
        bool else_ok = analyze_statement(analyzer, if_stmt->data.if_stmt.else_block);
        analyzer_exit_scope(analyzer);
        
        if (!else_ok) all_ok = false;
    }
    
    return all_ok;
}
```

### Análisis de Bucles

```c
static bool analyze_while_statement(SemanticAnalyzer *analyzer, AstNode *while_stmt) {
    // Analizar condición
    TypeInfo condition_type = analyze_expression(analyzer, while_stmt->data.while_stmt.condition);
    if (condition_type.base_type != TYPE_BOOLEAN) {
        semantic_error(analyzer, while_stmt->data.while_stmt.condition->span, 3030,
                      "Condición de While debe ser Boolean");
        return false;
    }
    
    // Analizar cuerpo en nuevo ámbito
    analyzer_enter_scope(analyzer, "while_body");
    bool body_ok = analyze_statement(analyzer, while_stmt->data.while_stmt.body);
    analyzer_exit_scope(analyzer);
    
    return body_ok;
}

static bool analyze_for_statement(SemanticAnalyzer *analyzer, AstNode *for_stmt) {
    // Entrar en ámbito del for (la inicialización es visible en todo el for)
    analyzer_enter_scope(analyzer, "for");
    
    bool all_ok = true;
    
    // Analizar inicialización
    if (!analyze_statement(analyzer, for_stmt->data.for_stmt.init)) {
        all_ok = false;
    }
    
    // Analizar condición
    TypeInfo condition_type = analyze_expression(analyzer, for_stmt->data.for_stmt.condition);
    if (condition_type.base_type != TYPE_BOOLEAN) {
        semantic_error(analyzer, for_stmt->data.for_stmt.condition->span, 3031,
                      "Condición de For debe ser Boolean");
        all_ok = false;
    }
    
    // Analizar actualización
    if (!analyze_statement(analyzer, for_stmt->data.for_stmt.update)) {
        all_ok = false;
    }
    
    // Analizar cuerpo
    if (!analyze_statement(analyzer, for_stmt->data.for_stmt.body)) {
        all_ok = false;
    }
    
    analyzer_exit_scope(analyzer);
    
    return all_ok;
}
```

---

## 🔄 Conversiones de Tipos

### Conversiones Automáticas Permitidas

```c
bool is_convertible_auto(TypeInfo from, TypeInfo to) {
    // Conversión de Integer a Float (promoción)
    if (from.base_type == TYPE_INTEGER && to.base_type == TYPE_FLOAT) {
        return true;
    }
    
    // Conversión de Char a String (concatenación)
    if (from.base_type == TYPE_CHAR && to.base_type == TYPE_STRING) {
        return true;
    }
    
    // Tipos idénticos siempre son convertibles
    if (types_equal(from, to)) {
        return true;
    }
    
    return false;
}
```

### Verificación de Compatibilidad

```c
bool types_compatible_for_assignment(TypeInfo target, TypeInfo source) {
    // Tipos exactamente iguales
    if (types_equal(target, source)) {
        return true;
    }
    
    // Conversiones automáticas permitidas
    if (is_convertible_auto(source, target)) {
        return true;
    }
    
    // Null puede asignarse a cualquier tipo (valor por defecto)
    if (source.base_type == TYPE_NULL) {
        return true;
    }
    
    return false;
}

bool types_compatible_for_comparison(TypeInfo left, TypeInfo right) {
    // Tipos numéricos son compatibles entre sí para comparación
    if (is_numeric_type(left) && is_numeric_type(right)) {
        return true;
    }
    
    // Tipos exactamente iguales
    if (types_equal(left, right)) {
        return true;
    }
    
    return false;
}
```

### Ejemplos de Conversiones

```c
// Conversiones automáticas válidas:
Integer x = 42;
Float y = x;        // ✅ Integer → Float (promoción)

Char c = 'A';
String s = c;       // ✅ Char → String (concatenación)

Integer z = Null;   // ✅ Null → cualquier tipo

// Conversiones inválidas:
String text = "Hello";
Integer num = text; // ❌ Error: String no es convertible a Integer

Boolean flag = True;
Float value = flag; // ❌ Error: Boolean no es convertible a Float
```

---

## ❌ Manejo de Errores Semánticos

### Códigos de Error Semántico (3000-3999)

```c
#define SEMANTIC_ERROR_UNKNOWN_TYPE         3001  // Tipo no reconocido
#define SEMANTIC_ERROR_MISSING_IDENTIFIER   3002  // Identificador faltante
#define SEMANTIC_ERROR_TYPE_MISMATCH        3003  // Tipos incompatibles
#define SEMANTIC_ERROR_REDECLARATION        3004  // Redeclaración
#define SEMANTIC_ERROR_INCOMPATIBLE_INIT    3005  // Inicializador incompatible
#define SEMANTIC_ERROR_INVALID_ARRAY_SIZE   3006  // Tamaño de array inválido
#define SEMANTIC_ERROR_UNSUPPORTED_EXPR     3007  // Expresión no soportada
#define SEMANTIC_ERROR_UNDECLARED_ID        3008  // Identificador no declarado
#define SEMANTIC_WARNING_UNINITIALIZED     3009  // Variable no inicializada
#define SEMANTIC_ERROR_UNKNOWN_FUNCTION     3010  // Función no declarada
#define SEMANTIC_ERROR_NOT_FUNCTION         3011  // No es función
#define SEMANTIC_ERROR_WRONG_ARG_COUNT      3012  // Número de argumentos incorrecto
#define SEMANTIC_ERROR_WRONG_ARG_TYPE       3013  // Tipo de argumento incorrecto
#define SEMANTIC_ERROR_INVALID_OPERATOR     3014  // Operador inválido
#define SEMANTIC_ERROR_NON_NUMERIC_OPERAND  3015  // Operando no numérico
#define SEMANTIC_ERROR_NON_BOOLEAN_OPERAND  3017  // Operando no booleano
#define SEMANTIC_ERROR_INCOMPATIBLE_COMPARE 3019  // Tipos incompatibles para comparación
#define SEMANTIC_ERROR_RETURN_OUTSIDE_FUNC  3024  // Return fuera de función
#define SEMANTIC_ERROR_VOID_RETURN_VALUE    3025  // Return con valor en función void
#define SEMANTIC_ERROR_WRONG_RETURN_TYPE    3026  // Tipo de retorno incorrecto
#define SEMANTIC_ERROR_MISSING_RETURN_VALUE 3027  // Return sin valor en función no-void
#define SEMANTIC_ERROR_NON_BOOLEAN_CONDITION 3028  // Condición no booleana
```

### Sistema de Reporte de Errores

```c
static void semantic_error(SemanticAnalyzer *analyzer, SrcSpan span, 
                          int code, const char *format, ...) {
    analyzer->error_count++;
    
    char message[512];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    DiagLocation loc = diag_location_create(
        span.line_start, span.col_start, 
        span.col_end - span.col_start, ""
    );
    
    diag_error(STAGE_SEMANTIC, code, loc, message);
}

static void semantic_warning(SemanticAnalyzer *analyzer, SrcSpan span,
                           int code, const char *format, ...) {
    analyzer->warning_count++;
    
    char message[512];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    DiagLocation loc = diag_location_create(
        span.line_start, span.col_start,
        span.col_end - span.col_start, ""
    );
    
    diag_warning(STAGE_SEMANTIC, code, loc, message);
}
```

### Recuperación de Errores

```c
// El analizador semántico continúa el análisis incluso tras errores
bool analyze_program(SemanticAnalyzer *analyzer, AstNode *program) {
    if (!program) return false;
    
    bool overall_success = true;
    
    // Analizar todas las sentencias del programa
    for (size_t i = 0; i < program->data.program.statement_count; i++) {
        AstNode *stmt = program->data.program.statements[i];
        
        // Manejar nodos de error del parser
        if (stmt->type == AST_ERROR) {
            analyzer->error_count++;
            continue; // Saltar nodos de error pero continuar
        }
        
        // Analizar sentencia normal
        if (!analyze_statement(analyzer, stmt)) {
            overall_success = false;
            // NO return, continuar con siguiente sentencia
        }
    }
    
    // El análisis se considera exitoso si no hay errores
    return (analyzer->error_count == 0);
}
```

---

## 💡 Ejemplos de Análisis

### Ejemplo 1: Programa Válido

```c
// Código fuente:
Integer x = 10;
Float y = x + 3.14;
Boolean result = y > 20.0;

// Análisis semántico:
1. Declarar x: Integer con valor 10 ✅
2. Declarar y: Float
   - Analizar x + 3.14:
     * x es Integer ✅
     * 3.14 es Float ✅ 
     * Integer + Float → Float ✅
   - Asignar Float a Float ✅
3. Declarar result: Boolean
   - Analizar y > 20.0:
     * y es Float ✅
     * 20.0 es Float ✅
     * Float > Float → Boolean ✅
   - Asignar Boolean a Boolean ✅

// Resultado: ✅ Sin errores, análisis exitoso
```

### Ejemplo 2: Error de Tipos

```c
// Código fuente:
Integer x = "texto";
String s = x + 5;

// Análisis semántico:
1. Declarar x: Integer
   - Valor inicial: "texto" es String
   - Integer ← String: ❌ tipos incompatibles
   
   // Error generado:
   {
     "type": "diagnostic",
     "stage": "semantic",
     "severity": "error", 
     "code": 3005,
     "line": 1,
     "column": 13,
     "message": "Tipo de inicializador incompatible: se esperaba 'Integer', se obtuvo 'String'"
   }

2. Declarar s: String  
   - Analizar x + 5:
     * x es Integer (a pesar del error anterior)
     * 5 es Integer
     * Integer + Integer → Integer
   - String ← Integer: ❌ tipos incompatibles
   
   // Error adicional generado:
   {
     "code": 3005,
     "message": "Tipo de inicializador incompatible: se esperaba 'String', se obtuvo 'Integer'"
   }

// Resultado: ❌ 2 errores detectados, análisis falla
```

### Ejemplo 3: Error de Variables No Declaradas

```c
// Código fuente:
x = 42;
Integer y = z + 10;

// Análisis semántico:
1. Asignación a x:
   - Buscar x en tabla de símbolos: ❌ no encontrado
   
   // Error:
   {
     "code": 3008,
     "message": "Identificador no declarado: 'x'"
   }

2. Declarar y: Integer
   - Analizar z + 10:
     * Buscar z: ❌ no encontrado
     
     // Error:
     {
       "code": 3008, 
       "message": "Identificador no declarado: 'z'"
     }
     
     * 10 es Integer ✅
     * UNKNOWN + Integer → UNKNOWN
   - Integer ← UNKNOWN: ❌ tipos incompatibles (error cascada)

// Resultado: ❌ 3 errores detectados (2 primarios + 1 cascada)
```

### Ejemplo 4: Función con Errores

```c
// Código fuente:
Integer suma(Integer a, Float b) {
    String resultado = a + b;
    Return resultado;
} End

// Análisis semántico:
1. Declarar función suma:
   - Tipo retorno: Integer ✅
   - Parámetros: Integer a, Float b ✅
   
2. Entrar en scope de función
   - Declarar a: Integer (inicializado) ✅
   - Declarar b: Float (inicializado) ✅

3. Analizar cuerpo:
   - Declarar resultado: String
     * Analizar a + b: 
       - Integer + Float → Float ✅
     * String ← Float: ❌ tipos incompatibles
     
     // Error:
     {
       "code": 3005,
       "message": "Tipo de inicializador incompatible: se esperaba 'String', se obtuvo 'Float'"
     }
     
   - Return resultado:
     * resultado es String
     * Función espera Integer
     * Integer ← String: ❌ tipos incompatibles
     
     // Error:
     {
       "code": 3026,
       "message": "Tipo de retorno incorrecto: se esperaba 'Integer', se obtuvo 'String'"
     }

// Resultado: ❌ 2 errores en función
```

---

## 🎯 Resumen de Capacidades

### ✅ **Completamente Implementado**

- **Sistema de tipos completo** con 10 tipos diferentes
- **Hash tables eficientes** para gestión de símbolos (O(1) promedio)
- **Ámbitos jerárquicos** con búsqueda en cadena de padres  
- **Validación exhaustiva** de declaraciones, expresiones y sentencias
- **Análisis de funciones** con parámetros y tipos de retorno
- **Verificación de operaciones** con reglas de precedencia y compatibilidad
- **Conversiones automáticas** según especificación del lenguaje
- **Estructuras de control** con validación de condiciones Boolean
- **Recuperación robusta** tras errores semánticos
- **Diagnósticos detallados** con códigos específicos 3000+
- **Manejo de AST dañado** por errores sintácticos

### 🚀 **Rendimiento y Calidad**

- **Eficiencia**: O(1) lookup de símbolos, O(n) análisis completo
- **Robustez**: Continúa análisis tras errores, detecta múltiples problemas
- **Precisión**: Información exacta de ubicación de errores
- **Escalabilidad**: Hash tables con rehashing automático
- **Mantenibilidad**: Código modular bien estructurado y documentado

### 🔗 **Integración con Pipeline**

- **Entrada**: AST del parser (incluso con nodos AST_ERROR)
- **Salida**: AST validado semánticamente para interpreter
- **Diagnósticos**: JSON estructurado integrado con sistema unificado
- **Error recovery**: Permite ejecución solo de código completamente válido

---

**El Analizador Semántico de Quimera está completamente implementado y proporciona validación exhaustiva de todas las reglas semánticas del lenguaje, con manejo robusto de errores y eficiencia optimizada.**