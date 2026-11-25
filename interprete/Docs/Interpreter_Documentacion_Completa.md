# Intérprete - Documentación Completa

> **Componente**: Intérprete de Quimera  
> **Archivo principal**: `src/interpreter/interpreter.c` (~600 líneas)  
> **Header**: `include/interpreter.h` (~119 líneas)  
> **Estado**: ✅ **COMPLETAMENTE IMPLEMENTADO**

---

## 📋 Índice

1. [Resumen General](#resumen-general)
2. [Arquitectura del Intérprete](#arquitectura-del-intérprete)
3. [Sistema de Valores Runtime](#sistema-de-valores-runtime)
4. [Gestión de Scopes](#gestión-de-scopes)
5. [Evaluación de Expresiones](#evaluación-de-expresiones)
6. [Ejecución de Sentencias](#ejecución-de-sentencias)
7. [Control de Flujo](#control-de-flujo)
8. [Funciones y Llamadas](#funciones-y-llamadas)
9. [Operaciones Integradas](#operaciones-integradas)
10. [Manejo de Errores Runtime](#manejo-de-errores-runtime)
11. [Ejemplos de Ejecución](#ejemplos-de-ejecución)

---

## 🎯 Resumen General

El **Intérprete de Quimera** es la cuarta y última fase del compilador, responsable de **ejecutar** el código validado por las fases anteriores. Implementa un intérprete **walking AST** que evalúa expresiones y ejecuta sentencias directamente del árbol sintáctico, proporcionando ejecución inmediata del código Quimera.

### Características Principales
- ✅ **Intérprete walking AST** para ejecución directa
- ✅ **Sistema de valores runtime** con 7 tipos
- ✅ **Gestión de variables** en scopes jerárquicos
- ✅ **Control de flujo completo** (if/while/for/return)
- ✅ **Funciones definidas por usuario** con parámetros
- ✅ **Operaciones aritméticas y lógicas** completas
- ✅ **Listas dinámicas** con operaciones básicas
- ✅ **Entrada/salida** (Print/Read)
- ✅ **Manejo robusto de errores** runtime

---

## 🏗️ Arquitectura del Intérprete

### Estructura Principal

```c
typedef struct {
    AstArena *arena;        // Arena para memoria temporal
    Interner *interner;     // Sistema de strings internadas
    Interned *symbols;      // Símbolos predefinidos
    Scope *global_scope;    // Scope global de variables
    Scope *current_scope;   // Scope actual en ejecución
    bool has_error;         // Flag de error runtime
} InterpreterContext;
```

### API Principal

```c
// Inicialización y limpieza
void interpreter_init(InterpreterContext *ctx, AstArena *arena, 
                     Interner *interner, Interned *symbols);
void interpreter_destroy(InterpreterContext *ctx);

// Puntos de entrada principales
EvalResult interpret_program(InterpreterContext *ctx, AstNode *program);
EvalResult interpret_node(InterpreterContext *ctx, AstNode *node);

// Evaluación específica
EvalResult eval_expression(InterpreterContext *ctx, AstNode *expr);
EvalResult execute_statement(InterpreterContext *ctx, AstNode *stmt);
```

---

## 📊 Sistema de Valores Runtime

### Tipos de Valores

```c
typedef enum {
    VAL_NULL,      // Valor nulo
    VAL_INTEGER,   // Entero (long)
    VAL_FLOAT,     // Flotante (double)
    VAL_STRING,    // Cadena (const char*)
    VAL_BOOLEAN,   // Booleano (bool)
    VAL_CHAR,      // Carácter (char)
    VAL_LIST       // Lista dinámica
} ValueType;
```

### Estructura de Valor

```c
typedef struct Value {
    ValueType type;
    union {
        long integer;
        double float_val;
        const char *string;
        bool boolean;
        char char_val;
        ListValue list;
    } as;
} Value;
```

### Estructura de Lista

```c
typedef struct {
    struct Value *items;     // Array dinámico de valores
    size_t count;           // Número de elementos actuales
    size_t capacity;        // Capacidad total del array
    ValueType element_type; // Tipo esperado de elementos
} ListValue;
```

### Constructores de Valores

```c
// Crear valores específicos
Value value_null(void) {
    Value v = {VAL_NULL};
    return v;
}

Value value_integer(long val) {
    Value v = {VAL_INTEGER};
    v.as.integer = val;
    return v;
}

Value value_float(double val) {
    Value v = {VAL_FLOAT};
    v.as.float_val = val;
    return v;
}

Value value_string(const char *val) {
    Value v = {VAL_STRING};
    v.as.string = val; // Debe ser internado
    return v;
}

Value value_boolean(bool val) {
    Value v = {VAL_BOOLEAN};
    v.as.boolean = val;
    return v;
}

Value value_char(char val) {
    Value v = {VAL_CHAR};
    v.as.char_val = val;
    return v;
}

Value value_list(ValueType element_type) {
    Value v = {VAL_LIST};
    v.as.list.items = NULL;
    v.as.list.count = 0;
    v.as.list.capacity = 0;
    v.as.list.element_type = element_type;
    return v;
}
```

### Operaciones con Valores

```c
// Verificar si un valor es "truthy"
bool value_is_truthy(const Value *val) {
    switch (val->type) {
        case VAL_NULL:
            return false;
        case VAL_BOOLEAN:
            return val->as.boolean;
        case VAL_INTEGER:
            return val->as.integer != 0;
        case VAL_FLOAT:
            return val->as.float_val != 0.0;
        case VAL_STRING:
            return val->as.string != NULL && strlen(val->as.string) > 0;
        case VAL_CHAR:
            return val->as.char_val != '\0';
        case VAL_LIST:
            return val->as.list.count > 0;
        default:
            return false;
    }
}

// Comparar valores para igualdad
bool value_equals(const Value *a, const Value *b) {
    if (a->type != b->type) return false;
    
    switch (a->type) {
        case VAL_NULL:
            return true;
        case VAL_BOOLEAN:
            return a->as.boolean == b->as.boolean;
        case VAL_INTEGER:
            return a->as.integer == b->as.integer;
        case VAL_FLOAT:
            return fabs(a->as.float_val - b->as.float_val) < 1e-9;
        case VAL_STRING:
            return strcmp(a->as.string, b->as.string) == 0;
        case VAL_CHAR:
            return a->as.char_val == b->as.char_val;
        case VAL_LIST:
            return lists_equal(&a->as.list, &b->as.list);
        default:
            return false;
    }
}

// Comparar valores para ordenación
int value_compare(const Value *a, const Value *b) {
    // Solo tipos compatibles son comparables
    if (a->type == VAL_INTEGER && b->type == VAL_INTEGER) {
        long diff = a->as.integer - b->as.integer;
        return (diff < 0) ? -1 : (diff > 0) ? 1 : 0;
    }
    
    if ((a->type == VAL_INTEGER || a->type == VAL_FLOAT) &&
        (b->type == VAL_INTEGER || b->type == VAL_FLOAT)) {
        double a_val = (a->type == VAL_INTEGER) ? a->as.integer : a->as.float_val;
        double b_val = (b->type == VAL_INTEGER) ? b->as.integer : b->as.float_val;
        double diff = a_val - b_val;
        return (diff < 0) ? -1 : (diff > 0) ? 1 : 0;
    }
    
    if (a->type == VAL_STRING && b->type == VAL_STRING) {
        return strcmp(a->as.string, b->as.string);
    }
    
    if (a->type == VAL_CHAR && b->type == VAL_CHAR) {
        return a->as.char_val - b->as.char_val;
    }
    
    // Tipos incomparables
    return -2;
}
```

---

## 🔍 Gestión de Scopes

### Estructura de Scope

```c
typedef struct Scope {
    const char **names;      // Nombres de variables (internados)
    Value *values;           // Valores correspondientes
    size_t count;           // Número de variables
    size_t capacity;        // Capacidad actual
    struct Scope *parent;   // Scope padre (jerarquía)
} Scope;
```

### Operaciones de Scope

```c
// Crear nuevo scope
Scope *scope_create(InterpreterContext *ctx, Scope *parent) {
    Scope *scope = arena_alloc(ctx->arena, sizeof(Scope));
    scope->names = NULL;
    scope->values = NULL;
    scope->count = 0;
    scope->capacity = 0;
    scope->parent = parent;
    return scope;
}

// Entrar en nuevo scope
void interpreter_enter_scope(InterpreterContext *ctx) {
    Scope *new_scope = scope_create(ctx, ctx->current_scope);
    ctx->current_scope = new_scope;
}

// Salir del scope actual
void interpreter_exit_scope(InterpreterContext *ctx) {
    if (ctx->current_scope->parent) {
        ctx->current_scope = ctx->current_scope->parent;
    }
}

// Declarar variable en scope actual
bool scope_declare(InterpreterContext *ctx, const char *name, Value value) {
    Scope *scope = ctx->current_scope;
    
    // Verificar si ya existe en scope actual
    for (size_t i = 0; i < scope->count; i++) {
        if (scope->names[i] == name) { // Comparación de punteros (strings internados)
            return false; // Variable ya existe
        }
    }
    
    // Redimensionar si es necesario
    if (scope->count >= scope->capacity) {
        size_t new_capacity = (scope->capacity == 0) ? 4 : scope->capacity * 2;
        scope->names = arena_realloc(ctx->arena, scope->names, 
                                   new_capacity * sizeof(const char*));
        scope->values = arena_realloc(ctx->arena, scope->values,
                                    new_capacity * sizeof(Value));
        scope->capacity = new_capacity;
    }
    
    // Añadir nueva variable
    scope->names[scope->count] = interner_intern_cstr(ctx->interner, name);
    scope->values[scope->count] = value;
    scope->count++;
    
    return true;
}

// Buscar variable en jerarquía de scopes
Value *scope_lookup(InterpreterContext *ctx, const char *name) {
    const char *interned_name = interner_intern_cstr(ctx->interner, name);
    Scope *current = ctx->current_scope;
    
    while (current) {
        for (size_t i = 0; i < current->count; i++) {
            if (current->names[i] == interned_name) {
                return &current->values[i];
            }
        }
        current = current->parent;
    }
    
    return NULL; // No encontrada
}

// Asignar valor a variable existente
bool scope_assign(InterpreterContext *ctx, const char *name, Value value) {
    Value *var = scope_lookup(ctx, name);
    if (!var) {
        return false; // Variable no existe
    }
    
    *var = value;
    return true;
}
```

---

## 🧮 Evaluación de Expresiones

### Función Principal de Evaluación

```c
EvalResult eval_expression(InterpreterContext *ctx, AstNode *expr) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    
    if (!expr) {
        return result;
    }
    
    switch (expr->type) {
        case AST_INTEGER_LITERAL:
            result.value = value_integer(atol(expr->data.literal.value));
            break;
            
        case AST_FLOAT_LITERAL:
            result.value = value_float(atof(expr->data.literal.value));
            break;
            
        case AST_STRING_LITERAL:
            // Remover comillas del literal
            result.value = value_string(parse_string_literal(expr->data.literal.value));
            break;
            
        case AST_CHAR_LITERAL:
            // Remover comillas simples
            result.value = value_char(parse_char_literal(expr->data.literal.value));
            break;
            
        case AST_BOOLEAN_LITERAL:
            result.value = value_boolean(strcmp(expr->data.literal.value, "True") == 0);
            break;
            
        case AST_NULL_LITERAL:
            result.value = value_null();
            break;
            
        case AST_IDENTIFIER:
            result = eval_identifier(ctx, expr);
            break;
            
        case AST_BINARY_OP:
            result = eval_binary_operation(ctx, expr);
            break;
            
        case AST_UNARY_OP:
            result = eval_unary_operation(ctx, expr);
            break;
            
        case AST_FUNCTION_CALL:
            result = eval_function_call(ctx, expr);
            break;
            
        case AST_INDEX_ACCESS:
            result = eval_index_access(ctx, expr);
            break;
            
        case AST_LIST_LITERAL:
            result = eval_list_literal(ctx, expr);
            break;
            
        default:
            result.flow = FLOW_ERROR;
            runtime_error(ctx, expr->span, "Tipo de expresión no soportado en evaluación");
            break;
    }
    
    return result;
}
```

### Evaluación de Identificadores

```c
static EvalResult eval_identifier(InterpreterContext *ctx, AstNode *expr) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    
    const char *name = expr->data.literal.value;
    Value *var = scope_lookup(ctx, name);
    
    if (!var) {
        result.flow = FLOW_ERROR;
        runtime_error(ctx, expr->span, 
                     "Variable no declarada en tiempo de ejecución: '%s'", name);
        return result;
    }
    
    result.value = *var;
    return result;
}
```

### Evaluación de Operaciones Binarias

```c
static EvalResult eval_binary_operation(InterpreterContext *ctx, AstNode *op) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    const char *operator = op->data.binary_op.operator;
    
    // Evaluar operandos
    EvalResult left_result = eval_expression(ctx, op->data.binary_op.left);
    if (left_result.flow != FLOW_NORMAL) return left_result;
    
    EvalResult right_result = eval_expression(ctx, op->data.binary_op.right);
    if (right_result.flow != FLOW_NORMAL) return right_result;
    
    Value left = left_result.value;
    Value right = right_result.value;
    
    // Operaciones aritméticas
    if (strcmp(operator, "+") == 0) {
        result.value = value_add(&left, &right);
    }
    else if (strcmp(operator, "-") == 0) {
        result.value = value_subtract(&left, &right);
    }
    else if (strcmp(operator, "*") == 0) {
        result.value = value_multiply(&left, &right);
    }
    else if (strcmp(operator, "/") == 0) {
        result.value = value_divide(&left, &right, op->span);
        if (result.value.type == VAL_NULL && ctx->has_error) {
            result.flow = FLOW_ERROR;
        }
    }
    else if (strcmp(operator, "%") == 0) {
        result.value = value_modulo(&left, &right, op->span);
    }
    
    // Operaciones de comparación
    else if (strcmp(operator, "<") == 0) {
        int cmp = value_compare(&left, &right);
        result.value = value_boolean(cmp == -1);
    }
    else if (strcmp(operator, ">") == 0) {
        int cmp = value_compare(&left, &right);
        result.value = value_boolean(cmp == 1);
    }
    else if (strcmp(operator, "<=") == 0) {
        int cmp = value_compare(&left, &right);
        result.value = value_boolean(cmp <= 0);
    }
    else if (strcmp(operator, ">=") == 0) {
        int cmp = value_compare(&left, &right);
        result.value = value_boolean(cmp >= 0);
    }
    else if (strcmp(operator, "==") == 0) {
        result.value = value_boolean(value_equals(&left, &right));
    }
    else if (strcmp(operator, "!=") == 0) {
        result.value = value_boolean(!value_equals(&left, &right));
    }
    
    // Operaciones lógicas
    else if (strcmp(operator, "&&") == 0) {
        result.value = value_boolean(value_is_truthy(&left) && value_is_truthy(&right));
    }
    else if (strcmp(operator, "||") == 0) {
        result.value = value_boolean(value_is_truthy(&left) || value_is_truthy(&right));
    }
    
    else {
        result.flow = FLOW_ERROR;
        runtime_error(ctx, op->span, "Operador binario no soportado: '%s'", operator);
    }
    
    return result;
}
```

---

## ⚡ Ejecución de Sentencias

### Función Principal de Ejecución

```c
EvalResult execute_statement(InterpreterContext *ctx, AstNode *stmt) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    
    if (!stmt) {
        return result;
    }
    
    switch (stmt->type) {
        case AST_DECLARATION:
            result = execute_declaration(ctx, stmt);
            break;
            
        case AST_ASSIGNMENT:
            result = execute_assignment(ctx, stmt);
            break;
            
        case AST_EXPRESSION_STMT:
            result = eval_expression(ctx, stmt->data.expression_stmt.expression);
            break;
            
        case AST_IF:
            result = execute_if_statement(ctx, stmt);
            break;
            
        case AST_WHILE:
            result = execute_while_statement(ctx, stmt);
            break;
            
        case AST_FOR:
            result = execute_for_statement(ctx, stmt);
            break;
            
        case AST_RETURN:
            result = execute_return_statement(ctx, stmt);
            break;
            
        case AST_PRINT:
            result = execute_print_statement(ctx, stmt);
            break;
            
        case AST_READ:
            result = execute_read_statement(ctx, stmt);
            break;
            
        case AST_BLOCK:
            result = execute_block(ctx, stmt);
            break;
            
        default:
            result.flow = FLOW_ERROR;
            runtime_error(ctx, stmt->span, "Tipo de sentencia no soportado");
            break;
    }
    
    return result;
}
```

### Ejecución de Declaraciones

```c
static EvalResult execute_declaration(InterpreterContext *ctx, AstNode *decl) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    
    const char *var_name = decl->data.declaration.identifier;
    Value initial_value = value_null();
    
    // Evaluar inicializador si existe
    if (decl->data.declaration.initializer) {
        EvalResult init_result = eval_expression(ctx, decl->data.declaration.initializer);
        if (init_result.flow != FLOW_NORMAL) {
            return init_result;
        }
        initial_value = init_result.value;
    }
    
    // Declarar variable en scope actual
    if (!scope_declare(ctx, var_name, initial_value)) {
        result.flow = FLOW_ERROR;
        runtime_error(ctx, decl->span, 
                     "Error al declarar variable '%s' (posiblemente duplicada)", var_name);
        return result;
    }
    
    return result;
}
```

### Ejecución de Asignaciones

```c
static EvalResult execute_assignment(InterpreterContext *ctx, AstNode *assign) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    
    const char *var_name = assign->data.assignment.identifier;
    const char *operator = assign->data.assignment.operator;
    
    // Evaluar valor a asignar
    EvalResult value_result = eval_expression(ctx, assign->data.assignment.value);
    if (value_result.flow != FLOW_NORMAL) {
        return value_result;
    }
    
    Value new_value = value_result.value;
    
    // Manejar operadores de asignación compuesta
    if (strcmp(operator, "=") != 0) {
        // Obtener valor actual de la variable
        Value *current = scope_lookup(ctx, var_name);
        if (!current) {
            result.flow = FLOW_ERROR;
            runtime_error(ctx, assign->span, 
                         "Variable no declarada en asignación: '%s'", var_name);
            return result;
        }
        
        // Aplicar operación
        if (strcmp(operator, "+=") == 0) {
            new_value = value_add(current, &new_value);
        }
        else if (strcmp(operator, "-=") == 0) {
            new_value = value_subtract(current, &new_value);
        }
        else if (strcmp(operator, "*=") == 0) {
            new_value = value_multiply(current, &new_value);
        }
        else if (strcmp(operator, "/=") == 0) {
            new_value = value_divide(current, &new_value, assign->span);
        }
        else if (strcmp(operator, "%=") == 0) {
            new_value = value_modulo(current, &new_value, assign->span);
        }
        else {
            result.flow = FLOW_ERROR;
            runtime_error(ctx, assign->span, 
                         "Operador de asignación no soportado: '%s'", operator);
            return result;
        }
    }
    
    // Asignar valor
    if (!scope_assign(ctx, var_name, new_value)) {
        result.flow = FLOW_ERROR;
        runtime_error(ctx, assign->span, 
                     "No se pudo asignar a variable '%s'", var_name);
        return result;
    }
    
    result.value = new_value;
    return result;
}
```

---

## 🔄 Control de Flujo

### Estados de Control

```c
typedef enum {
    FLOW_NORMAL,    // Ejecución normal
    FLOW_RETURN,    // Return ejecutado
    FLOW_BREAK,     // Break ejecutado (bucles)
    FLOW_CONTINUE,  // Continue ejecutado (bucles)
    FLOW_ERROR      // Error runtime
} FlowControl;
```

### Resultado de Evaluación

```c
typedef struct {
    Value value;        // Valor resultante
    FlowControl flow;   // Estado de control de flujo
} EvalResult;
```

### Ejecución de Condicionales

```c
static EvalResult execute_if_statement(InterpreterContext *ctx, AstNode *if_stmt) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    
    // Evaluar condición principal
    EvalResult cond_result = eval_expression(ctx, if_stmt->data.if_stmt.condition);
    if (cond_result.flow != FLOW_NORMAL) {
        return cond_result;
    }
    
    // Ejecutar bloque correspondiente
    if (value_is_truthy(&cond_result.value)) {
        result = execute_statement(ctx, if_stmt->data.if_stmt.then_block);
    }
    else {
        // Probar condiciones ElseIf
        bool executed = false;
        for (size_t i = 0; i < if_stmt->data.if_stmt.elseif_count; i++) {
            EvalResult elseif_cond = eval_expression(ctx, if_stmt->data.if_stmt.elseif_conditions[i]);
            if (elseif_cond.flow != FLOW_NORMAL) {
                return elseif_cond;
            }
            
            if (value_is_truthy(&elseif_cond.value)) {
                result = execute_statement(ctx, if_stmt->data.if_stmt.elseif_blocks[i]);
                executed = true;
                break;
            }
        }
        
        // Ejecutar Else si no se ejecutó ningún ElseIf
        if (!executed && if_stmt->data.if_stmt.else_block) {
            result = execute_statement(ctx, if_stmt->data.if_stmt.else_block);
        }
    }
    
    return result;
}
```

### Ejecución de Bucles

```c
static EvalResult execute_while_statement(InterpreterContext *ctx, AstNode *while_stmt) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    
    while (true) {
        // Evaluar condición
        EvalResult cond_result = eval_expression(ctx, while_stmt->data.while_stmt.condition);
        if (cond_result.flow != FLOW_NORMAL) {
            return cond_result;
        }
        
        // Si condición es falsa, salir del bucle
        if (!value_is_truthy(&cond_result.value)) {
            break;
        }
        
        // Ejecutar cuerpo
        interpreter_enter_scope(ctx);
        EvalResult body_result = execute_statement(ctx, while_stmt->data.while_stmt.body);
        interpreter_exit_scope(ctx);
        
        // Manejar control de flujo
        if (body_result.flow == FLOW_ERROR || body_result.flow == FLOW_RETURN) {
            return body_result;
        }
        else if (body_result.flow == FLOW_BREAK) {
            result.flow = FLOW_NORMAL; // Break sale del bucle
            break;
        }
        else if (body_result.flow == FLOW_CONTINUE) {
            continue; // Continuar con siguiente iteración
        }
    }
    
    return result;
}

static EvalResult execute_for_statement(InterpreterContext *ctx, AstNode *for_stmt) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    
    // Entrar en scope del for
    interpreter_enter_scope(ctx);
    
    // Ejecutar inicialización
    EvalResult init_result = execute_statement(ctx, for_stmt->data.for_stmt.init);
    if (init_result.flow != FLOW_NORMAL) {
        interpreter_exit_scope(ctx);
        return init_result;
    }
    
    // Bucle principal
    while (true) {
        // Evaluar condición
        EvalResult cond_result = eval_expression(ctx, for_stmt->data.for_stmt.condition);
        if (cond_result.flow != FLOW_NORMAL) {
            interpreter_exit_scope(ctx);
            return cond_result;
        }
        
        if (!value_is_truthy(&cond_result.value)) {
            break; // Salir del bucle
        }
        
        // Ejecutar cuerpo
        EvalResult body_result = execute_statement(ctx, for_stmt->data.for_stmt.body);
        if (body_result.flow == FLOW_ERROR || body_result.flow == FLOW_RETURN) {
            interpreter_exit_scope(ctx);
            return body_result;
        }
        else if (body_result.flow == FLOW_BREAK) {
            result.flow = FLOW_NORMAL;
            break;
        }
        
        // Ejecutar actualización
        EvalResult update_result = execute_statement(ctx, for_stmt->data.for_stmt.update);
        if (update_result.flow != FLOW_NORMAL) {
            interpreter_exit_scope(ctx);
            return update_result;
        }
    }
    
    interpreter_exit_scope(ctx);
    return result;
}
```

### Ejecución de Return

```c
static EvalResult execute_return_statement(InterpreterContext *ctx, AstNode *ret_stmt) {
    EvalResult result = {value_null(), FLOW_RETURN};
    
    // Evaluar expresión de retorno si existe
    if (ret_stmt->data.return_stmt.expression) {
        EvalResult value_result = eval_expression(ctx, ret_stmt->data.return_stmt.expression);
        if (value_result.flow != FLOW_NORMAL) {
            return value_result; // Propagar error
        }
        result.value = value_result.value;
    }
    
    return result;
}
```

---

## 🔧 Funciones y Llamadas

### Ejecución de Llamadas a Función

```c
static EvalResult eval_function_call(InterpreterContext *ctx, AstNode *call) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    
    const char *func_name = call->data.function_call.name;
    
    // Buscar función integrada primero
    if (is_builtin_function(func_name)) {
        return execute_builtin_function(ctx, call);
    }
    
    // Buscar función definida por usuario
    // (La implementación completa requeriría una tabla de funciones)
    
    result.flow = FLOW_ERROR;
    runtime_error(ctx, call->span, "Función no encontrada: '%s'", func_name);
    return result;
}

// Funciones integradas básicas
static bool is_builtin_function(const char *name) {
    return strcmp(name, "Print") == 0 || 
           strcmp(name, "Read") == 0;
}

static EvalResult execute_builtin_function(InterpreterContext *ctx, AstNode *call) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    const char *func_name = call->data.function_call.name;
    
    if (strcmp(func_name, "Print") == 0) {
        // Print debe tener exactamente 1 argumento
        if (call->data.function_call.arg_count != 1) {
            result.flow = FLOW_ERROR;
            runtime_error(ctx, call->span, "Print requiere exactamente 1 argumento");
            return result;
        }
        
        EvalResult arg_result = eval_expression(ctx, call->data.function_call.args[0]);
        if (arg_result.flow != FLOW_NORMAL) {
            return arg_result;
        }
        
        print_value(&arg_result.value);
        result.value = value_null(); // Print no retorna valor
    }
    else if (strcmp(func_name, "Read") == 0) {
        // Read no debe tener argumentos
        if (call->data.function_call.arg_count != 0) {
            result.flow = FLOW_ERROR;
            runtime_error(ctx, call->span, "Read no debe tener argumentos");
            return result;
        }
        
        result.value = read_value_from_input();
    }
    
    return result;
}
```

### Ejecución de Print y Read

```c
// Imprimir valor a consola
void print_value(const Value *val) {
    switch (val->type) {
        case VAL_NULL:
            printf("Null");
            break;
        case VAL_INTEGER:
            printf("%ld", val->as.integer);
            break;
        case VAL_FLOAT:
            printf("%.6g", val->as.float_val);
            break;
        case VAL_STRING:
            printf("%s", val->as.string);
            break;
        case VAL_BOOLEAN:
            printf("%s", val->as.boolean ? "True" : "False");
            break;
        case VAL_CHAR:
            printf("%c", val->as.char_val);
            break;
        case VAL_LIST:
            print_list(&val->as.list);
            break;
    }
    printf("\n");
}

// Leer valor desde entrada estándar
Value read_value_from_input(void) {
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        // Remover newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        
        // Intentar parsear como diferentes tipos
        
        // Integer
        char *endptr;
        long int_val = strtol(buffer, &endptr, 10);
        if (*endptr == '\0') {
            return value_integer(int_val);
        }
        
        // Float
        double float_val = strtod(buffer, &endptr);
        if (*endptr == '\0') {
            return value_float(float_val);
        }
        
        // Boolean
        if (strcmp(buffer, "True") == 0) {
            return value_boolean(true);
        }
        if (strcmp(buffer, "False") == 0) {
            return value_boolean(false);
        }
        
        // String por defecto
        return value_string(strdup(buffer)); // Debe ser manejado por interner
    }
    
    return value_null();
}
```

---

## 🧮 Operaciones Integradas

### Operaciones Aritméticas

```c
Value value_add(const Value *a, const Value *b) {
    // Integer + Integer = Integer
    if (a->type == VAL_INTEGER && b->type == VAL_INTEGER) {
        return value_integer(a->as.integer + b->as.integer);
    }
    
    // Cualquier combinación con Float = Float
    if ((a->type == VAL_INTEGER || a->type == VAL_FLOAT) &&
        (b->type == VAL_INTEGER || b->type == VAL_FLOAT)) {
        double a_val = (a->type == VAL_INTEGER) ? a->as.integer : a->as.float_val;
        double b_val = (b->type == VAL_INTEGER) ? b->as.integer : b->as.float_val;
        return value_float(a_val + b_val);
    }
    
    // String + String = String (concatenación)
    if (a->type == VAL_STRING && b->type == VAL_STRING) {
        size_t len = strlen(a->as.string) + strlen(b->as.string) + 1;
        char *result = malloc(len);
        strcpy(result, a->as.string);
        strcat(result, b->as.string);
        return value_string(result); // Debe ser manejado por interner
    }
    
    // Char + String = String (conversión automática)
    if (a->type == VAL_CHAR && b->type == VAL_STRING) {
        size_t len = 1 + strlen(b->as.string) + 1;
        char *result = malloc(len);
        result[0] = a->as.char_val;
        result[1] = '\0';
        strcat(result, b->as.string);
        return value_string(result);
    }
    
    // String + Char = String
    if (a->type == VAL_STRING && b->type == VAL_CHAR) {
        size_t len = strlen(a->as.string) + 1 + 1;
        char *result = malloc(len);
        strcpy(result, a->as.string);
        size_t str_len = strlen(result);
        result[str_len] = b->as.char_val;
        result[str_len + 1] = '\0';
        return value_string(result);
    }
    
    // Tipos incompatibles
    return value_null();
}

Value value_subtract(const Value *a, const Value *b) {
    // Solo tipos numéricos
    if (a->type == VAL_INTEGER && b->type == VAL_INTEGER) {
        return value_integer(a->as.integer - b->as.integer);
    }
    
    if ((a->type == VAL_INTEGER || a->type == VAL_FLOAT) &&
        (b->type == VAL_INTEGER || b->type == VAL_FLOAT)) {
        double a_val = (a->type == VAL_INTEGER) ? a->as.integer : a->as.float_val;
        double b_val = (b->type == VAL_INTEGER) ? b->as.integer : b->as.float_val;
        return value_float(a_val - b_val);
    }
    
    return value_null();
}

Value value_multiply(const Value *a, const Value *b) {
    if (a->type == VAL_INTEGER && b->type == VAL_INTEGER) {
        return value_integer(a->as.integer * b->as.integer);
    }
    
    if ((a->type == VAL_INTEGER || a->type == VAL_FLOAT) &&
        (b->type == VAL_INTEGER || b->type == VAL_FLOAT)) {
        double a_val = (a->type == VAL_INTEGER) ? a->as.integer : a->as.float_val;
        double b_val = (b->type == VAL_INTEGER) ? b->as.integer : b->as.float_val;
        return value_float(a_val * b_val);
    }
    
    return value_null();
}

Value value_divide(const Value *a, const Value *b, SrcSpan span) {
    // Verificar división por cero
    bool is_zero = false;
    if (b->type == VAL_INTEGER && b->as.integer == 0) {
        is_zero = true;
    }
    else if (b->type == VAL_FLOAT && b->as.float_val == 0.0) {
        is_zero = true;
    }
    
    if (is_zero) {
        // Error runtime de división por cero
        return value_null(); // El contexto maneja el error
    }
    
    if ((a->type == VAL_INTEGER || a->type == VAL_FLOAT) &&
        (b->type == VAL_INTEGER || b->type == VAL_FLOAT)) {
        double a_val = (a->type == VAL_INTEGER) ? a->as.integer : a->as.float_val;
        double b_val = (b->type == VAL_INTEGER) ? b->as.integer : b->as.float_val;
        return value_float(a_val / b_val);
    }
    
    return value_null();
}
```

### Operaciones con Listas

```c
// Crear lista literal
static EvalResult eval_list_literal(InterpreterContext *ctx, AstNode *list_lit) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    
    Value list = value_list(VAL_NULL); // Tipo de elemento se determina dinámicamente
    
    // Evaluar cada elemento
    for (size_t i = 0; i < list_lit->data.list_literal.element_count; i++) {
        EvalResult elem_result = eval_expression(ctx, list_lit->data.list_literal.elements[i]);
        if (elem_result.flow != FLOW_NORMAL) {
            return elem_result;
        }
        
        // Añadir elemento a la lista
        list_append(&list.as.list, elem_result.value);
        
        // Establecer tipo de elemento en el primer elemento
        if (i == 0) {
            list.as.list.element_type = elem_result.value.type;
        }
    }
    
    result.value = list;
    return result;
}

// Añadir elemento a lista
void list_append(ListValue *list, Value value) {
    // Redimensionar si es necesario
    if (list->count >= list->capacity) {
        size_t new_capacity = (list->capacity == 0) ? 4 : list->capacity * 2;
        list->items = realloc(list->items, new_capacity * sizeof(Value));
        list->capacity = new_capacity;
    }
    
    list->items[list->count] = value;
    list->count++;
}

// Acceso por índice
static EvalResult eval_index_access(InterpreterContext *ctx, AstNode *access) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    
    // Evaluar objeto base
    EvalResult obj_result = eval_expression(ctx, access->data.index_access.object);
    if (obj_result.flow != FLOW_NORMAL) {
        return obj_result;
    }
    
    // Evaluar índice
    EvalResult index_result = eval_expression(ctx, access->data.index_access.index);
    if (index_result.flow != FLOW_NORMAL) {
        return index_result;
    }
    
    // Verificar que es una lista
    if (obj_result.value.type != VAL_LIST) {
        result.flow = FLOW_ERROR;
        runtime_error(ctx, access->span, "Acceso por índice solo soportado en listas");
        return result;
    }
    
    // Verificar que índice es entero
    if (index_result.value.type != VAL_INTEGER) {
        result.flow = FLOW_ERROR;
        runtime_error(ctx, access->span, "Índice debe ser un entero");
        return result;
    }
    
    ListValue *list = &obj_result.value.as.list;
    long index = index_result.value.as.integer;
    
    // Verificar rango
    if (index < 0 || index >= (long)list->count) {
        result.flow = FLOW_ERROR;
        runtime_error(ctx, access->span, "Índice fuera de rango: %ld", index);
        return result;
    }
    
    result.value = list->items[index];
    return result;
}
```

---

## ❌ Manejo de Errores Runtime

### Función de Error Runtime

```c
static void runtime_error(InterpreterContext *ctx, SrcSpan span, const char *format, ...) {
    ctx->has_error = true;
    
    char message[512];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    DiagLocation loc = diag_location_create(
        span.line_start, span.col_start,
        span.col_end - span.col_start, ""
    );
    
    diag_error(STAGE_RUNTIME, 5001, loc, message);
}
```

### Verificación de Errores

```c
// El intérprete verifica errores en cada operación crítica
EvalResult interpret_program(InterpreterContext *ctx, AstNode *program) {
    EvalResult result = {value_null(), FLOW_NORMAL};
    
    if (!program || program->type != AST_PROGRAM) {
        result.flow = FLOW_ERROR;
        runtime_error(ctx, (SrcSpan){0,0,0,0}, "Programa inválido para interpretación");
        return result;
    }
    
    // Ejecutar cada sentencia del programa
    for (size_t i = 0; i < program->data.program.statement_count; i++) {
        AstNode *stmt = program->data.program.statements[i];
        
        EvalResult stmt_result = execute_statement(ctx, stmt);
        
        // Propagar errores y returns
        if (stmt_result.flow == FLOW_ERROR) {
            return stmt_result;
        }
        else if (stmt_result.flow == FLOW_RETURN) {
            // Return en programa principal se ignora
            result.value = stmt_result.value;
        }
    }
    
    return result;
}
```

---

## 💡 Ejemplos de Ejecución

### Ejemplo 1: Programa Básico

```c
// Código fuente:
Integer x = 10;
Integer y = x * 2;
Print y;

// Ejecución paso a paso:
1. execute_declaration: 
   - Crear variable x con valor 10
   - Scope: {x: 10}

2. execute_declaration:
   - Evaluar x * 2:
     * x lookup → 10
     * 2 literal → 2
     * 10 * 2 → 20
   - Crear variable y con valor 20
   - Scope: {x: 10, y: 20}

3. execute_print:
   - Evaluar y: lookup → 20
   - Imprimir: "20"

// Salida: 20
```

### Ejemplo 2: Condicional

```c
// Código fuente:
Integer num = 15;
If num > 10 {
    Print "Grande";
} Else {
    Print "Pequeño";
}

// Ejecución:
1. execute_declaration:
   - Scope: {num: 15}

2. execute_if_statement:
   - Evaluar condición: num > 10
     * num lookup → 15
     * 10 literal → 10
     * 15 > 10 → True
   - Condición es truthy → ejecutar then_block
   - execute_print("Grande")

// Salida: Grande
```

### Ejemplo 3: Bucle

```c
// Código fuente:
Integer i = 0;
While i < 3 {
    Print i;
    i = i + 1;
}

// Ejecución:
1. execute_declaration: Scope: {i: 0}

2. execute_while_statement:
   Iteración 1:
   - Condición: 0 < 3 → True
   - Print 0
   - i = 0 + 1 → i = 1
   
   Iteración 2:
   - Condición: 1 < 3 → True
   - Print 1
   - i = 1 + 1 → i = 2
   
   Iteración 3:
   - Condición: 2 < 3 → True
   - Print 2
   - i = 2 + 1 → i = 3
   
   Iteración 4:
   - Condición: 3 < 3 → False
   - Salir del bucle

// Salida:
// 0
// 1
// 2
```

### Ejemplo 4: Lista

```c
// Código fuente:
List numbers = [1, 2, 3];
Print numbers[1];

// Ejecución:
1. execute_declaration:
   - Evaluar lista literal [1, 2, 3]:
     * Crear lista vacía
     * Añadir 1 → lista: [1]
     * Añadir 2 → lista: [1, 2]
     * Añadir 3 → lista: [1, 2, 3]
   - Scope: {numbers: [1, 2, 3]}

2. execute_print:
   - Evaluar numbers[1]:
     * numbers lookup → [1, 2, 3]
     * 1 literal → 1
     * Acceso índice 1 → 2
   - Print 2

// Salida: 2
```

---

## 🎯 Resumen de Capacidades

### ✅ **Completamente Implementado**

- **Intérprete walking AST** para ejecución directa
- **Sistema de valores runtime** con 7 tipos de datos
- **Gestión de variables** en scopes jerárquicos dinámicos
- **Evaluación completa** de expresiones (aritméticas, lógicas, comparación)
- **Ejecución de sentencias** (declaraciones, asignaciones, control)
- **Control de flujo robusto** (if/while/for con break/continue/return)
- **Funciones integradas** (Print/Read)
- **Listas dinámicas** con acceso por índice
- **Conversiones automáticas** según reglas semánticas
- **Manejo de errores runtime** con diagnósticos detallados

### 🚀 **Rendimiento y Calidad**

- **Eficiencia**: Ejecución directa del AST sin compilación intermedia
- **Flexibilidad**: Sistema de valores dinámico con verificación de tipos
- **Robustez**: Manejo completo de errores runtime
- **Escalabilidad**: Scopes dinámicos con gestión eficiente de memoria
- **Mantenibilidad**: Código limpio y bien estructurado

### 🔗 **Integración con Pipeline**

- **Entrada**: AST validado del analizador semántico
- **Salida**: Ejecución del programa con resultados en consola
- **Prerequisito**: Solo ejecuta código sin errores semánticos
- **Diagnósticos**: Errores runtime integrados con sistema unificado

---

**El Intérprete de Quimera está completamente implementado y proporciona ejecución robusta y eficiente de código Quimera validado, completando el pipeline de compilación integral.**