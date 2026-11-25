#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "interner.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Tipos de valores en tiempo de ejecución */
typedef enum {
    VAL_NULL,
    VAL_INTEGER,
    VAL_FLOAT,
    VAL_STRING,
    VAL_BOOLEAN,
    VAL_CHAR,
    VAL_LIST
} ValueType;

/* Arreglos y listas: objetos por referencia (como en Java o Python), así `l.append(x)` modifica
   la variable y pasarlos a una función no los copia. El intérprete los libera todos al terminar. */
typedef struct ListObj {
    struct Value *items;
    size_t count;
    size_t capacity;
    bool fixed;                  // arreglo: tamaño fijo, sin append/remove
    struct ListObj *next_alloc;  // cadena de todas las listas creadas, para liberarlas
} ListObj;

/* Valor en tiempo de ejecución */
typedef struct Value {
    ValueType type;
    union {
        long integer;
        double float_val;
        const char *string;
        bool boolean;
        char char_val;
        ListObj *list;
    } as;
} Value;

/* Estados de control de flujo */
typedef enum {
    FLOW_NORMAL,
    FLOW_RETURN,
    FLOW_BREAK,
    FLOW_CONTINUE,
    FLOW_ERROR
} FlowControl;

/* Resultado de evaluación con control de flujo */
typedef struct {
    Value value;
    FlowControl flow;
} EvalResult;

/* Scope/Environment para variables */
typedef struct Scope {
    const char **names;      // nombres de variables (internados)
    Value *values;           // valores correspondientes
    const char **types;      // tipo declarado de cada variable (Read lo usa si el valor es Null)
    size_t count;
    size_t capacity;
    struct Scope *parent;    // scope padre
} Scope;

/* Función declarada (nombre internado -> nodo AST_FUNC_DECL) */
typedef struct {
    const char *name;
    struct AstNode *decl;
} FunctionEntry;

/* Context del intérprete */
typedef struct {
    AstArena *arena;        // para memoria temporal
    Interner *interner;     // para strings internados
    Interned *symbols;      // símbolos predefinidos
    Scope *global_scope;    // scope global
    Scope *current_scope;   // scope actual
    bool has_error;         // flag de error
    FunctionEntry *functions;   // funciones declaradas
    size_t function_count, function_capacity;
    ListObj *lists;             // todas las listas/arreglos creados
    int call_depth;             // llamadas anidadas en curso (límite contra recursión infinita)
} InterpreterContext;

/* API principal */
void interpreter_init(InterpreterContext *ctx, AstArena *arena, 
                     Interner *interner, Interned *symbols);
void interpreter_destroy(InterpreterContext *ctx);

EvalResult interpret_program(InterpreterContext *ctx, AstNode *program);
EvalResult interpret_node(InterpreterContext *ctx, AstNode *node);

/* Utilidades para valores */
Value value_null(void);
Value value_integer(long val);
Value value_float(double val);
Value value_string(const char *val);
Value value_boolean(bool val);
Value value_char(char val);
Value value_list(ListObj *list);

/* Texto de un valor tal como lo muestra Print (listas como [1, 2, 3]) */
void value_format(const Value *val, char *buf, size_t cap);

bool value_is_truthy(const Value *val);
bool value_equals(const Value *a, const Value *b);
int value_compare(const Value *a, const Value *b); // -1, 0, 1 o -2 (incomparable)
Value value_add(const Value *a, const Value *b);
Value value_subtract(const Value *a, const Value *b);
Value value_multiply(const Value *a, const Value *b);
Value value_divide(const Value *a, const Value *b, SrcSpan span);
Value value_modulo(const Value *a, const Value *b, SrcSpan span);
Value value_negate(const Value *val);
Value value_not(const Value *val);

void value_print(const Value *val);
const char* value_type_name(ValueType type);

/* Scope management */
Scope* scope_new(Scope *parent);
void scope_destroy(Scope *scope);
void scope_define(Scope *scope, const char *name, Value value);
Value* scope_lookup(Scope *scope, const char *name);
bool scope_assign(Scope *scope, const char *name, Value value);
void scope_set_type(Scope *scope, const char *name, const char *type_name);
const char *scope_lookup_type(Scope *scope, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* INTERPRETER_H */