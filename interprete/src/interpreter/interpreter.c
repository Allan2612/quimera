#include "interpreter.h"
#include "symbol_table.h"
#include "diag.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ============ Utilidades de valores ============ */

Value value_null(void) {
    Value v = { .type = VAL_NULL };
    return v;
}

Value value_integer(long val) {
    Value v = { .type = VAL_INTEGER, .as.integer = val };
    return v;
}

Value value_float(double val) {
    Value v = { .type = VAL_FLOAT, .as.float_val = val };
    return v;
}

Value value_string(const char *val) {
    Value v = { .type = VAL_STRING, .as.string = val };
    return v;
}

Value value_boolean(bool val) {
    Value v = { .type = VAL_BOOLEAN, .as.boolean = val };
    return v;
}

Value value_char(char val) {
    Value v = { .type = VAL_CHAR, .as.char_val = val };
    return v;
}

Value value_list(ListObj *list) {
    Value v = { .type = VAL_LIST, .as.list = list };
    return v;
}

/* Los literales guardan sus comillas ("\"Hola\"") y la concatenación no: comparar el contenido */
static const char *string_content(const char *s, size_t *len) {
    size_t n = strlen(s);
    if (n >= 2 && s[0] == '"' && s[n - 1] == '"') {
        *len = n - 2;
        return s + 1;
    }
    *len = n;
    return s;
}

static int string_compare(const char *a, const char *b) {
    size_t la, lb;
    const char *ca = string_content(a, &la);
    const char *cb = string_content(b, &lb);
    int c = memcmp(ca, cb, la < lb ? la : lb);
    if (c != 0) return c;
    return (la > lb) - (la < lb);
}

const char* value_type_name(ValueType type) {
    switch (type) {
        case VAL_NULL: return "Null";
        case VAL_INTEGER: return "Integer";
        case VAL_FLOAT: return "Float";
        case VAL_STRING: return "String";
        case VAL_BOOLEAN: return "Boolean";
        case VAL_CHAR: return "Char";
        case VAL_LIST: return "List";
        default: return "Unknown";
    }
}

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
            return val->as.list && val->as.list->count > 0;
        default:
            return true;
    }
}

bool value_equals(const Value *a, const Value *b) {
    if (a->type != b->type) {
        // Permitir comparación entre Char y String
        if ((a->type == VAL_CHAR && b->type == VAL_STRING) ||
            (a->type == VAL_STRING && b->type == VAL_CHAR)) {
            if (a->type == VAL_CHAR) {
                return strlen(b->as.string) == 1 && b->as.string[0] == a->as.char_val;
            } else {
                return strlen(a->as.string) == 1 && a->as.string[0] == b->as.char_val;
            }
        }
        return false;
    }

    switch (a->type) {
        case VAL_NULL:
            return true;
        case VAL_INTEGER:
            return a->as.integer == b->as.integer;
        case VAL_FLOAT:
            return a->as.float_val == b->as.float_val;
        case VAL_STRING:
            return string_compare(a->as.string, b->as.string) == 0;
        case VAL_BOOLEAN:
            return a->as.boolean == b->as.boolean;
        case VAL_CHAR:
            return a->as.char_val == b->as.char_val;
        case VAL_LIST:
            // Misma lista (referencia)
            return a->as.list == b->as.list;
        default:
            return false;
    }
}

int value_compare(const Value *a, const Value *b) {
    // Promoción Integer → Float si es necesario
    if ((a->type == VAL_INTEGER && b->type == VAL_FLOAT) ||
        (a->type == VAL_FLOAT && b->type == VAL_INTEGER)) {
        
        double da = (a->type == VAL_INTEGER) ? (double)a->as.integer : a->as.float_val;
        double db = (b->type == VAL_INTEGER) ? (double)b->as.integer : b->as.float_val;
        
        if (da < db) return -1;
        if (da > db) return 1;
        return 0;
    }

    // Comparación entre tipos idénticos
    if (a->type != b->type) {
        return -2; // Incomparable
    }

    switch (a->type) {
        case VAL_INTEGER:
            if (a->as.integer < b->as.integer) return -1;
            if (a->as.integer > b->as.integer) return 1;
            return 0;
        case VAL_FLOAT:
            if (a->as.float_val < b->as.float_val) return -1;
            if (a->as.float_val > b->as.float_val) return 1;
            return 0;
        case VAL_STRING: {
            int c = string_compare(a->as.string, b->as.string);
            return (c > 0) - (c < 0);
        }
        case VAL_CHAR:
            if (a->as.char_val < b->as.char_val) return -1;
            if (a->as.char_val > b->as.char_val) return 1;
            return 0;
        default:
            return -2; // Incomparable
    }
}

Value value_add(const Value *a, const Value *b) {
    // Null handling: Null + anything = 0 or ""
    if (a->type == VAL_NULL) {
        if (b->type == VAL_STRING) return value_string("");
        return value_integer(0);
    }
    if (b->type == VAL_NULL) {
        if (a->type == VAL_STRING) return value_string("");
        return value_integer(0);
    }

    // Char con String: el Char pasa a ser un texto de un carácter
    if ((a->type == VAL_CHAR && b->type == VAL_STRING) ||
        (a->type == VAL_STRING && b->type == VAL_CHAR)) {
        char ca[2] = { a->type == VAL_CHAR ? a->as.char_val : '\0', '\0' };
        char cb[2] = { b->type == VAL_CHAR ? b->as.char_val : '\0', '\0' };
        Value sa = a->type == VAL_CHAR ? value_string(ca) : *a;
        Value sb = b->type == VAL_CHAR ? value_string(cb) : *b;
        return value_add(&sa, &sb);  /* la concatenación copia el resultado */
    }

    // Promoción numérica
    if ((a->type == VAL_INTEGER && b->type == VAL_FLOAT) ||
        (a->type == VAL_FLOAT && b->type == VAL_INTEGER)) {
        double da = (a->type == VAL_INTEGER) ? (double)a->as.integer : a->as.float_val;
        double db = (b->type == VAL_INTEGER) ? (double)b->as.integer : b->as.float_val;
        return value_float(da + db);
    }

    switch (a->type) {
        case VAL_INTEGER:
            if (b->type == VAL_INTEGER) {
                return value_integer(a->as.integer + b->as.integer);
            }
            break;
        case VAL_FLOAT:
            if (b->type == VAL_FLOAT) {
                return value_float(a->as.float_val + b->as.float_val);
            }
            break;
        case VAL_STRING:
            if (b->type == VAL_STRING) {
                // Concatenación usando arena para mejor gestión de memoria
                size_t len_a = strlen(a->as.string);
                size_t len_b = strlen(b->as.string);
                
                // Quitar comillas si están presentes
                const char *str_a = a->as.string;
                const char *str_b = b->as.string;
                size_t start_a = 0, end_a = len_a;
                size_t start_b = 0, end_b = len_b;
                
                if (len_a >= 2 && str_a[0] == '"' && str_a[len_a-1] == '"') {
                    start_a = 1;
                    end_a = len_a - 1;
                }
                if (len_b >= 2 && str_b[0] == '"' && str_b[len_b-1] == '"') {
                    start_b = 1;
                    end_b = len_b - 1;
                }
                
                size_t final_len = (end_a - start_a) + (end_b - start_b);
                char *result = malloc(final_len + 1);
                if (!result) return value_string("");
                
                memcpy(result, str_a + start_a, end_a - start_a);
                memcpy(result + (end_a - start_a), str_b + start_b, end_b - start_b);
                result[final_len] = '\0';
                
                return value_string(result);
            }
            // Char → String conversion
            if (b->type == VAL_CHAR) {
                size_t len = strlen(a->as.string) + 2;
                char *result = malloc(len);
                strcpy(result, a->as.string);
                result[strlen(a->as.string)] = b->as.char_val;
                result[len-1] = '\0';
                return value_string(result);
            }
            break;
        case VAL_CHAR:
            if (b->type == VAL_STRING) {
                size_t len = strlen(b->as.string) + 2;
                char *result = malloc(len);
                result[0] = a->as.char_val;
                strcpy(result + 1, b->as.string);
                return value_string(result);
            }
            break;
        case VAL_NULL:
        case VAL_BOOLEAN: 
        case VAL_LIST:
            // Operaciones no soportadas para estos tipos
            break;
    }
    
    // Error: operación inválida
    return value_integer(0);
}

Value value_subtract(const Value *a, const Value *b) {
    // Null handling
    if (a->type == VAL_NULL || b->type == VAL_NULL) {
        return value_integer(0);
    }

    // Promoción numérica
    if ((a->type == VAL_INTEGER && b->type == VAL_FLOAT) ||
        (a->type == VAL_FLOAT && b->type == VAL_INTEGER)) {
        double da = (a->type == VAL_INTEGER) ? (double)a->as.integer : a->as.float_val;
        double db = (b->type == VAL_INTEGER) ? (double)b->as.integer : b->as.float_val;
        return value_float(da - db);
    }

    if (a->type == VAL_INTEGER && b->type == VAL_INTEGER) {
        return value_integer(a->as.integer - b->as.integer);
    }
    if (a->type == VAL_FLOAT && b->type == VAL_FLOAT) {
        return value_float(a->as.float_val - b->as.float_val);
    }
    
    return value_integer(0);
}

Value value_multiply(const Value *a, const Value *b) {
    // Null handling
    if (a->type == VAL_NULL || b->type == VAL_NULL) {
        return value_integer(0);
    }

    // Promoción numérica
    if ((a->type == VAL_INTEGER && b->type == VAL_FLOAT) ||
        (a->type == VAL_FLOAT && b->type == VAL_INTEGER)) {
        double da = (a->type == VAL_INTEGER) ? (double)a->as.integer : a->as.float_val;
        double db = (b->type == VAL_INTEGER) ? (double)b->as.integer : b->as.float_val;
        return value_float(da * db);
    }

    if (a->type == VAL_INTEGER && b->type == VAL_INTEGER) {
        return value_integer(a->as.integer * b->as.integer);
    }
    if (a->type == VAL_FLOAT && b->type == VAL_FLOAT) {
        return value_float(a->as.float_val * b->as.float_val);
    }
    
    return value_integer(0);
}

Value value_divide(const Value *a, const Value *b, SrcSpan span) {
    // Null handling
    if (a->type == VAL_NULL || b->type == VAL_NULL) {
        return value_integer(0);
    }

    // Check division by zero
    bool is_zero = false;
    if (b->type == VAL_INTEGER) {
        is_zero = (b->as.integer == 0);
    } else if (b->type == VAL_FLOAT) {
        is_zero = (b->as.float_val == 0.0);
    }

    if (is_zero) {
        DiagLocation loc = diag_location_simple(span.line_start, span.col_start);
        diag_error(STAGE_RUNTIME, 4001, loc, NULL);
        return value_integer(0);
    }

    // Promoción numérica
    if ((a->type == VAL_INTEGER && b->type == VAL_FLOAT) ||
        (a->type == VAL_FLOAT && b->type == VAL_INTEGER)) {
        double da = (a->type == VAL_INTEGER) ? (double)a->as.integer : a->as.float_val;
        double db = (b->type == VAL_INTEGER) ? (double)b->as.integer : b->as.float_val;
        return value_float(da / db);
    }

    if (a->type == VAL_INTEGER && b->type == VAL_INTEGER) {
        return value_integer(a->as.integer / b->as.integer);
    }
    if (a->type == VAL_FLOAT && b->type == VAL_FLOAT) {
        return value_float(a->as.float_val / b->as.float_val);
    }
    
    return value_integer(0);
}

Value value_modulo(const Value *a, const Value *b, SrcSpan span) {
    // Solo para enteros
    if (a->type != VAL_INTEGER || b->type != VAL_INTEGER) {
        return value_integer(0);
    }

    if (b->as.integer == 0) {
        DiagLocation loc = diag_location_simple(span.line_start, span.col_start);
        diag_error(STAGE_RUNTIME, 4001, loc, "Módulo por cero");
        return value_integer(0);
    }

    return value_integer(a->as.integer % b->as.integer);
}

Value value_negate(const Value *val) {
    switch (val->type) {
        case VAL_INTEGER:
            return value_integer(-val->as.integer);
        case VAL_FLOAT:
            return value_float(-val->as.float_val);
        case VAL_NULL:
            return value_integer(0);
        default:
            return value_integer(0);
    }
}

Value value_not(const Value *val) {
    return value_boolean(!value_is_truthy(val));
}

/* Agrega texto a buf sin pasarse de cap (si no cabe, termina en "...") */
static void buf_append(char *buf, size_t cap, size_t *pos, const char *text, size_t n) {
    if (*pos >= cap - 1) return;
    size_t room = cap - 1 - *pos;
    if (n > room) {
        memcpy(buf + *pos, text, room);
        *pos += room;
        if (cap >= 4) memcpy(buf + cap - 4, "...", 3);
    } else {
        memcpy(buf + *pos, text, n);
        *pos += n;
    }
    buf[*pos] = '\0';
}

static void value_format_into(const Value *val, char *buf, size_t cap, size_t *pos) {
    char tmp[64];
    switch (val->type) {
        case VAL_NULL: buf_append(buf, cap, pos, "null", 4); break;
        case VAL_INTEGER: {
            int n = snprintf(tmp, sizeof(tmp), "%ld", val->as.integer);
            buf_append(buf, cap, pos, tmp, (size_t)n);
            break;
        }
        case VAL_FLOAT: {
            int n = snprintf(tmp, sizeof(tmp), "%g", val->as.float_val);
            buf_append(buf, cap, pos, tmp, (size_t)n);
            break;
        }
        case VAL_STRING: {
            size_t n;
            const char *c = string_content(val->as.string, &n);
            buf_append(buf, cap, pos, c, n);
            break;
        }
        case VAL_BOOLEAN:
            buf_append(buf, cap, pos, val->as.boolean ? "true" : "false", val->as.boolean ? 4 : 5);
            break;
        case VAL_CHAR:
            if (val->as.char_val) buf_append(buf, cap, pos, &val->as.char_val, 1);
            break;
        case VAL_LIST: {
            buf_append(buf, cap, pos, "[", 1);
            const ListObj *l = val->as.list;
            for (size_t i = 0; l && i < l->count; i++) {
                if (i > 0) buf_append(buf, cap, pos, ", ", 2);
                value_format_into(&l->items[i], buf, cap, pos);
            }
            buf_append(buf, cap, pos, "]", 1);
            break;
        }
        default: buf_append(buf, cap, pos, "<unknown>", 9); break;
    }
}

void value_format(const Value *val, char *buf, size_t cap) {
    if (!buf || cap == 0) return;
    size_t pos = 0;
    buf[0] = '\0';
    value_format_into(val, buf, cap, &pos);
}

void value_print(const Value *val) {
    char buffer[512];
    
    switch (val->type) {
        case VAL_NULL:
            output_program_value("null", "null");
            break;
        case VAL_INTEGER:
            snprintf(buffer, sizeof(buffer), "%ld", val->as.integer);
            output_program_value(buffer, "integer");
            break;
        case VAL_FLOAT:
            snprintf(buffer, sizeof(buffer), "%g", val->as.float_val);
            output_program_value(buffer, "float");
            break;
        case VAL_STRING: {
            const char *str = val->as.string;
            size_t len = strlen(str);
            // Si la string tiene comillas, quitarlas para la impresión
            if (len >= 2 && str[0] == '"' && str[len-1] == '"') {
                // Crear una copia sin las comillas
                char *unquoted = malloc(len - 1);
                if (unquoted) {
                    memcpy(unquoted, str + 1, len - 2);
                    unquoted[len - 2] = '\0';
                    output_program_value(unquoted, "string");
                    free(unquoted);
                } else {
                    output_program_value(str, "string");
                }
            } else {
                output_program_value(str, "string");
            }
            break;
        }
        case VAL_BOOLEAN:
            output_program_value(val->as.boolean ? "true" : "false", "boolean");
            break;
        case VAL_CHAR:
            snprintf(buffer, sizeof(buffer), "%c", val->as.char_val);
            output_program_value(buffer, "char");
            break;
        case VAL_LIST:
            value_format(val, buffer, sizeof(buffer));
            output_program_value(buffer, "list");
            break;
        default:
            output_program_value("<unknown>", "unknown");
            break;
    }
}

/* ============ Scope management ============ */

Scope* scope_new(Scope *parent) {
    Scope *scope = calloc(1, sizeof(Scope));
    if (!scope) return NULL;
    
    scope->parent = parent;
    scope->capacity = 8;
    scope->names = calloc(scope->capacity, sizeof(const char*));
    scope->values = calloc(scope->capacity, sizeof(Value));
    scope->types = calloc(scope->capacity, sizeof(const char*));

    if (!scope->names || !scope->values || !scope->types) {
        free(scope->names);
        free(scope->values);
        free(scope->types);
        free(scope);
        return NULL;
    }
    
    return scope;
}

void scope_destroy(Scope *scope) {
    if (!scope) return;
    free(scope->names);
    free(scope->values);
    free(scope->types);
    free(scope);
}

void scope_define(Scope *scope, const char *name, Value value) {
    if (!scope) return;

    // Verificar si ya existe en este scope
    for (size_t i = 0; i < scope->count; i++) {
        if (scope->names[i] == name) { // comparación por puntero (internado)
            scope->values[i] = value;
            return;
        }
    }

    // Redimensionar si es necesario
    if (scope->count >= scope->capacity) {
        size_t new_cap = scope->capacity * 2;
        const char **new_names = realloc(scope->names, new_cap * sizeof(const char*));
        if (new_names) scope->names = new_names;
        Value *new_values = realloc(scope->values, new_cap * sizeof(Value));
        if (new_values) scope->values = new_values;
        const char **new_types = realloc(scope->types, new_cap * sizeof(const char*));
        if (new_types) scope->types = new_types;

        if (!new_names || !new_values || !new_types) return;
        
        scope->capacity = new_cap;
    }

    scope->names[scope->count] = name;
    scope->values[scope->count] = value;
    scope->types[scope->count] = NULL;
    scope->count++;
}

void scope_set_type(Scope *scope, const char *name, const char *type_name) {
    if (!scope) return;
    for (size_t i = 0; i < scope->count; i++) {
        if (scope->names[i] == name) {
            scope->types[i] = type_name;
            return;
        }
    }
}

const char *scope_lookup_type(Scope *scope, const char *name) {
    for (Scope *s = scope; s; s = s->parent) {
        for (size_t i = 0; i < s->count; i++) {
            if (s->names[i] == name) return s->types[i];
        }
    }
    return NULL;
}

Value* scope_lookup(Scope *scope, const char *name) {
    for (Scope *s = scope; s; s = s->parent) {
        for (size_t i = 0; i < s->count; i++) {
            if (s->names[i] == name) { // comparación por puntero
                return &s->values[i];
            }
        }
    }
    return NULL;
}

bool scope_assign(Scope *scope, const char *name, Value value) {
    for (Scope *s = scope; s; s = s->parent) {
        for (size_t i = 0; i < s->count; i++) {
            if (s->names[i] == name) {
                s->values[i] = value;
                return true;
            }
        }
    }
    return false;
}

/* ============ Interpreter Context ============ */

void interpreter_init(InterpreterContext *ctx, AstArena *arena, 
                     Interner *interner, Interned *symbols) {
    ctx->arena = arena;
    ctx->interner = interner;
    ctx->symbols = symbols;
    ctx->global_scope = scope_new(NULL);
    ctx->current_scope = ctx->global_scope;
    ctx->has_error = false;
    ctx->functions = NULL;
    ctx->function_count = 0;
    ctx->function_capacity = 0;
    ctx->lists = NULL;
    ctx->call_depth = 0;
}

void interpreter_destroy(InterpreterContext *ctx) {
    Scope *scope = ctx->global_scope;
    while (scope) {
        Scope *next = scope->parent;
        scope_destroy(scope);
        scope = next;
    }
    ctx->global_scope = NULL;
    ctx->current_scope = NULL;

    free(ctx->functions);
    ctx->functions = NULL;
    ctx->function_count = ctx->function_capacity = 0;

    for (ListObj *l = ctx->lists; l;) {
        ListObj *next = l->next_alloc;
        free(l->items);
        free(l);
        l = next;
    }
    ctx->lists = NULL;
}

/* ============ Evaluación de nodos ============ */

/* Definidas más abajo, junto a funciones, arreglos y Read */
static EvalResult runtime_error(InterpreterContext *ctx, AstNode *node, int code, const char *msg);
static Value default_value_for(const char *type_name);
static EvalResult eval_collection_decl(InterpreterContext *ctx, AstNode *node, const char *type_name,
                                       bool is_array);
static EvalResult resolve_index(InterpreterContext *ctx, AstNode *node, Value **slot);

static EvalResult eval_literal(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL };
    (void)ctx;
    
    AstLiteral *lit = &node->as.literal;
    switch (lit->tag) {
        case LIT_INT:
            result.value = value_integer(lit->val.i);
            break;
        case LIT_FLOAT:
            result.value = value_float(lit->val.f);
            break;
        case LIT_STRING:
            result.value = value_string(lit->val.s);
            break;
        case LIT_BOOL:
            result.value = value_boolean(lit->val.b);
            break;
        case LIT_NULL:
            result.value = value_null();
            break;
        case LIT_CHAR:
            result.value = value_char((char)lit->val.i);
            break;
    }
    
    return result;
}

static EvalResult eval_identifier(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL };
    
    const char *name = node->as.ident.lexeme;
    const char *interned_name = interner_intern_cstr(ctx->interner, name);
    
    Value *val = scope_lookup(ctx->current_scope, interned_name);
    if (!val) {
        DiagLocation loc = diag_location_create(node->span.line_start, node->span.col_start, strlen(name), name);
        diag_error(STAGE_SEMANTIC, 3001, loc, NULL);
        result.value = value_null();
        result.flow = FLOW_ERROR;
        ctx->has_error = true;
    } else {
        result.value = *val;
    }
    
    return result;
}

static EvalResult eval_binary_op(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL };
    
    OpCode op = node->as.binop.op;
    
    // Short-circuit para And/Or
    if (op == OP_AND) {
        EvalResult left_result = interpret_node(ctx, node->as.binop.left);
        if (left_result.flow != FLOW_NORMAL) return left_result;
        
        if (!value_is_truthy(&left_result.value)) {
            result.value = value_boolean(false);
            return result;
        }
        
        EvalResult right_result = interpret_node(ctx, node->as.binop.right);
        if (right_result.flow != FLOW_NORMAL) return right_result;
        
        result.value = value_boolean(value_is_truthy(&right_result.value));
        return result;
    }
    
    if (op == OP_OR) {
        EvalResult left_result = interpret_node(ctx, node->as.binop.left);
        if (left_result.flow != FLOW_NORMAL) return left_result;
        
        if (value_is_truthy(&left_result.value)) {
            result.value = value_boolean(true);
            return result;
        }
        
        EvalResult right_result = interpret_node(ctx, node->as.binop.right);
        if (right_result.flow != FLOW_NORMAL) return right_result;
        
        result.value = value_boolean(value_is_truthy(&right_result.value));
        return result;
    }
    
    // Evaluar ambos operandos para otros operadores
    EvalResult left_result = interpret_node(ctx, node->as.binop.left);
    if (left_result.flow != FLOW_NORMAL) return left_result;
    
    EvalResult right_result = interpret_node(ctx, node->as.binop.right);
    if (right_result.flow != FLOW_NORMAL) return right_result;
    
    Value *left = &left_result.value;
    Value *right = &right_result.value;
    
    switch (op) {
        case OP_ADD:
            result.value = value_add(left, right);
            break;
        case OP_SUB:
            result.value = value_subtract(left, right);
            break;
        case OP_MUL:
            result.value = value_multiply(left, right);
            break;
        case OP_DIV:
        case OP_MOD:
            if ((right->type == VAL_INTEGER && right->as.integer == 0) ||
                (right->type == VAL_FLOAT && right->as.float_val == 0.0)) {
                return runtime_error(ctx, node, 4001, NULL);
            }
            result.value = op == OP_DIV ? value_divide(left, right, node->span)
                                        : value_modulo(left, right, node->span);
            break;
        case OP_EQ:
            result.value = value_boolean(value_equals(left, right));
            break;
        case OP_NEQ:
            result.value = value_boolean(!value_equals(left, right));
            break;
        case OP_LT:
        case OP_LTE:
        case OP_GT:
        case OP_GTE: {
            int cmp = value_compare(left, right);
            if (cmp == -2) {
                // Tipos incomparables
                DiagLocation loc = diag_location_simple(node->span.line_start, node->span.col_start);
                diag_warning(STAGE_SEMANTIC, 3003, loc, "Comparación entre tipos incompatibles");
                result.value = value_boolean(false);
            } else {
                bool comparison_result = false;
                switch (op) {
                    case OP_LT: comparison_result = (cmp < 0); break;
                    case OP_LTE: comparison_result = (cmp <= 0); break;
                    case OP_GT: comparison_result = (cmp > 0); break;
                    case OP_GTE: comparison_result = (cmp >= 0); break;
                    default: break;
                }
                result.value = value_boolean(comparison_result);
            }
            break;
        }
        default:
            result.value = value_null();
            break;
    }
    
    return result;
}

static EvalResult eval_unary_op(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL };
    
    EvalResult operand_result = interpret_node(ctx, node->as.unop.expr);
    if (operand_result.flow != FLOW_NORMAL) return operand_result;
    
    switch (node->as.unop.op) {
        case OP_NEG:
            result.value = value_negate(&operand_result.value);
            break;
        case OP_NOT:
            result.value = value_not(&operand_result.value);
            break;
        default:
            result.value = value_null();
            break;
    }
    
    return result;
}

static EvalResult eval_assignment(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL };
    
    // Asignación a un elemento: a[i] = x, a[i] += x
    if (node->as.assign.lhs->kind == AST_INDEX) {
        /* Primero el lado derecho: puede agregar elementos y mover la memoria de la lista */
        EvalResult rhs = interpret_node(ctx, node->as.assign.rhs);
        if (rhs.flow != FLOW_NORMAL) return rhs;
        Value *slot = NULL;
        EvalResult target = resolve_index(ctx, node->as.assign.lhs, &slot);
        if (target.flow != FLOW_NORMAL) return target;
        Value final_value = rhs.value;
        if (node->as.assign.op == OP_ADD_ASSIGN) final_value = value_add(slot, &rhs.value);
        else if (node->as.assign.op == OP_SUB_ASSIGN) final_value = value_subtract(slot, &rhs.value);
        *slot = final_value;
        result.value = final_value;
        return result;
    }

    if (node->as.assign.lhs->kind != AST_IDENT) {
        return runtime_error(ctx, node, 3004, NULL);
    }
    
    const char *name = node->as.assign.lhs->as.ident.lexeme;
    const char *interned_name = interner_intern_cstr(ctx->interner, name);
    
    EvalResult rhs_result = interpret_node(ctx, node->as.assign.rhs);
    if (rhs_result.flow != FLOW_NORMAL) return rhs_result;
    
    // Para asignaciones compuestas, obtener el valor actual
    Value final_value = rhs_result.value;
    if (node->as.assign.op != OP_ASSIGN) {
        Value *current = scope_lookup(ctx->current_scope, interned_name);
        if (!current) {
            DiagLocation loc = diag_location_create(node->span.line_start, node->span.col_start, strlen(name), name);
            diag_error(STAGE_SEMANTIC, 3001, loc, NULL);
            result.flow = FLOW_ERROR;
            return result;
        }
        
        switch (node->as.assign.op) {
            case OP_ADD_ASSIGN:
                final_value = value_add(current, &rhs_result.value);
                break;
            case OP_SUB_ASSIGN:
                final_value = value_subtract(current, &rhs_result.value);
                break;
            default:
                break;
        }
    }
    
    if (!scope_assign(ctx->current_scope, interned_name, final_value)) {
        DiagLocation loc = diag_location_create(node->span.line_start, node->span.col_start, strlen(name), name);
        diag_error(STAGE_SEMANTIC, 3001, loc, NULL);
        result.flow = FLOW_ERROR;
    } else {
        result.value = final_value;
    }
    
    return result;
}

static EvalResult eval_var_decl(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL };
    
    const char *type_name = node->as.var_decl.type_name;
    if (type_name && strncmp(type_name, "Array:", 6) == 0) {
        return eval_collection_decl(ctx, node, type_name, true);
    }
    if (type_name && strncmp(type_name, "List:", 5) == 0) {
        return eval_collection_decl(ctx, node, type_name, false);
    }

    const char *name = interner_intern_cstr(ctx->interner, node->as.var_decl.name);
    /* Sin valor inicial queda en Null (Docs/Semantica.md); se guarda el tipo para Read */
    Value init_value = value_null();
    
    if (node->as.var_decl.init) {
        EvalResult init_result = interpret_node(ctx, node->as.var_decl.init);
        if (init_result.flow != FLOW_NORMAL) return init_result;
        init_value = init_result.value;
    }
    
    scope_define(ctx->current_scope, name, init_value);
    scope_set_type(ctx->current_scope, name, type_name);
    result.value = init_value;
    
    return result;
}

static EvalResult eval_print(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL };
    
    // Para múltiples valores, crear un string concatenado
    char output_buffer[2048] = "";
    size_t buffer_pos = 0;
    
    for (size_t i = 0; i < node->as.print.exprs.count; i++) {
        if (i > 0 && buffer_pos < sizeof(output_buffer) - 1) {
            output_buffer[buffer_pos++] = ' ';
        }
        
        EvalResult expr_result = interpret_node(ctx, node->as.print.exprs.data[i]);
        if (expr_result.flow != FLOW_NORMAL) return expr_result;
        
        // Convertir el valor a texto (listas como [1, 2, 3]) y agregarlo al buffer
        char value_str[1024];
        value_format(&expr_result.value, value_str, sizeof(value_str));
        
        // Agregar al buffer
        size_t value_len = strlen(value_str);
        if (buffer_pos + value_len < sizeof(output_buffer) - 1) {
            memcpy(output_buffer + buffer_pos, value_str, value_len);
            buffer_pos += value_len;
        }
    }
    output_buffer[buffer_pos] = '\0';
    
    // Enviar toda la línea como salida del programa
    output_program_text(output_buffer);
    
    result.value = value_null();
    return result;
}

static EvalResult eval_if(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL };
    
    // Evaluar condición principal
    EvalResult cond_result = interpret_node(ctx, node->as.if_stmt.cond);
    if (cond_result.flow != FLOW_NORMAL) return cond_result;
    
    if (value_is_truthy(&cond_result.value)) {
        return interpret_node(ctx, node->as.if_stmt.then_blk);
    }
    
    // Evaluar ElseIfs
    for (size_t i = 0; i < node->as.if_stmt.elseifs.count; i++) {
        AstNode *elseif = node->as.if_stmt.elseifs.data[i];
        EvalResult elseif_cond = interpret_node(ctx, elseif->as.if_stmt.cond);
        if (elseif_cond.flow != FLOW_NORMAL) return elseif_cond;
        
        if (value_is_truthy(&elseif_cond.value)) {
            return interpret_node(ctx, elseif->as.if_stmt.then_blk);
        }
    }
    
    // Evaluar Else si existe
    if (node->as.if_stmt.else_blk) {
        return interpret_node(ctx, node->as.if_stmt.else_blk);
    }
    
    result.value = value_null();
    return result;
}

static EvalResult eval_while(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL, .value = value_null() };
    
    while (true) {
        EvalResult cond_result = interpret_node(ctx, node->as.while_stmt.cond);
        if (cond_result.flow != FLOW_NORMAL) return cond_result;
        
        if (!value_is_truthy(&cond_result.value)) {
            break;
        }
        
        EvalResult body_result = interpret_node(ctx, node->as.while_stmt.body);
        if (body_result.flow == FLOW_BREAK) {
            break;
        }
        if (body_result.flow == FLOW_CONTINUE) {
            continue;
        }
        if (body_result.flow != FLOW_NORMAL) {
            return body_result;
        }
    }
    
    return result;
}

static EvalResult eval_for(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL, .value = value_null() };
    
    // Crear nuevo scope para el for
    Scope *new_scope = scope_new(ctx->current_scope);
    if (!new_scope) {
        result.flow = FLOW_ERROR;
        return result;
    }
    
    Scope *prev_scope = ctx->current_scope;
    ctx->current_scope = new_scope;
    
    // Ejecutar inicialización
    if (node->as.for_stmt.init) {
        EvalResult init_result = interpret_node(ctx, node->as.for_stmt.init);
        if (init_result.flow != FLOW_NORMAL) {
            ctx->current_scope = prev_scope;
            scope_destroy(new_scope);
            return init_result;
        }
    }
    
    // Ciclo principal: condición -> cuerpo -> actualización
    while (true) {
        // Evaluar condición
        if (node->as.for_stmt.cond) {
            EvalResult cond_result = interpret_node(ctx, node->as.for_stmt.cond);
            if (cond_result.flow != FLOW_NORMAL) {
                ctx->current_scope = prev_scope;
                scope_destroy(new_scope);
                return cond_result;
            }
            
            if (!value_is_truthy(&cond_result.value)) {
                break;
            }
        }
        
        // Ejecutar cuerpo
        if (node->as.for_stmt.body) {
            EvalResult body_result = interpret_node(ctx, node->as.for_stmt.body);
            if (body_result.flow == FLOW_BREAK) {
                break;
            }
            if (body_result.flow == FLOW_CONTINUE) {
                // Continuar a la actualización
            } else if (body_result.flow != FLOW_NORMAL) {
                ctx->current_scope = prev_scope;
                scope_destroy(new_scope);
                return body_result;
            }
        }
        
        // Ejecutar actualización
        if (node->as.for_stmt.step) {
            EvalResult step_result = interpret_node(ctx, node->as.for_stmt.step);
            if (step_result.flow != FLOW_NORMAL) {
                ctx->current_scope = prev_scope;
                scope_destroy(new_scope);
                return step_result;
            }
        }
    }
    
    // Restaurar scope
    ctx->current_scope = prev_scope;
    scope_destroy(new_scope);
    
    return result;
}

static EvalResult eval_block(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL, .value = value_null() };
    
    // Crear nuevo scope
    Scope *new_scope = scope_new(ctx->current_scope);
    if (!new_scope) {
        result.flow = FLOW_ERROR;
        return result;
    }
    
    Scope *prev_scope = ctx->current_scope;
    ctx->current_scope = new_scope;
    
    // Ejecutar todas las sentencias del bloque
    for (size_t i = 0; i < node->as.block.stmts.count; i++) {
        EvalResult stmt_result = interpret_node(ctx, node->as.block.stmts.data[i]);
        
        if (stmt_result.flow != FLOW_NORMAL) {
            result = stmt_result;
            break;
        }
    }
    
    // Restaurar scope anterior
    ctx->current_scope = prev_scope;
    scope_destroy(new_scope);
    
    return result;
}

static EvalResult eval_return(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_RETURN };
    
    if (node->as.ret.expr) {
        EvalResult expr_result = interpret_node(ctx, node->as.ret.expr);
        if (expr_result.flow != FLOW_NORMAL) return expr_result;
        result.value = expr_result.value;
    } else {
        result.value = value_null();
    }
    
    return result;
}

/* ============ Errores de ejecución, valores por defecto y listas ============ */

/* Límites para que un programa no tumbe al intérprete (sobre todo en el navegador) */
#define MAX_CALL_DEPTH 400
#define MAX_ARRAY_SIZE 1000000
#define MAX_READ_LINE 1024

static EvalResult runtime_error(InterpreterContext *ctx, AstNode *node, int code, const char *msg) {
    DiagLocation loc = diag_location_simple(node ? node->span.line_start : 0, node ? node->span.col_start : 0);
    diag_error(STAGE_RUNTIME, code, loc, msg);
    ctx->has_error = true;
    EvalResult r = { .flow = FLOW_ERROR, .value = value_null() };
    return r;
}

/* Valor inicial de una variable declarada sin valor (Read necesita saber el tipo) */
static Value default_value_for(const char *type_name) {
    if (!type_name) return value_null();
    if (strcmp(type_name, "Integer") == 0) return value_integer(0);
    if (strcmp(type_name, "Float") == 0) return value_float(0.0);
    if (strcmp(type_name, "Boolean") == 0) return value_boolean(false);
    if (strcmp(type_name, "String") == 0) return value_string("");
    if (strcmp(type_name, "Char") == 0) return value_char('\0');
    return value_null();
}

static ListObj *list_new(InterpreterContext *ctx, size_t capacity, bool fixed) {
    ListObj *l = calloc(1, sizeof(ListObj));
    if (!l) return NULL;
    if (capacity > 0) {
        l->items = calloc(capacity, sizeof(Value));
        if (!l->items) {
            free(l);
            return NULL;
        }
    }
    l->capacity = capacity;
    l->fixed = fixed;
    l->next_alloc = ctx->lists;
    ctx->lists = l;
    return l;
}

static bool list_push_value(ListObj *l, Value v) {
    if (l->count >= l->capacity) {
        size_t cap = l->capacity ? l->capacity * 2 : 4;
        Value *items = realloc(l->items, cap * sizeof(Value));
        if (!items) return false;
        l->items = items;
        l->capacity = cap;
    }
    l->items[l->count++] = v;
    return true;
}

/* ============ Funciones ============ */

static AstNode *find_function(InterpreterContext *ctx, const char *name) {
    for (size_t i = 0; i < ctx->function_count; i++) {
        if (ctx->functions[i].name == name) return ctx->functions[i].decl;
    }
    return NULL;
}

static EvalResult eval_func_decl(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL, .value = value_null() };
    const char *name = interner_intern_cstr(ctx->interner, node->as.func_decl.name);
    for (size_t i = 0; i < ctx->function_count; i++) {
        if (ctx->functions[i].name == name) {
            ctx->functions[i].decl = node;
            return result;
        }
    }
    if (ctx->function_count >= ctx->function_capacity) {
        size_t cap = ctx->function_capacity ? ctx->function_capacity * 2 : 8;
        FunctionEntry *f = realloc(ctx->functions, cap * sizeof(FunctionEntry));
        if (!f) return runtime_error(ctx, node, 5002, NULL);
        ctx->functions = f;
        ctx->function_capacity = cap;
    }
    ctx->functions[ctx->function_count].name = name;
    ctx->functions[ctx->function_count].decl = node;
    ctx->function_count++;
    return result;
}

static EvalResult call_user_function(InterpreterContext *ctx, AstNode *call, AstNode *fn) {
    AstList *params = &fn->as.func_decl.params;
    AstList *args = &call->as.call.args;

    if (params->count != args->count) {
        char msg[160];
        snprintf(msg, sizeof(msg), "'%s' espera %zu argumento(s) y recibió %zu",
                 fn->as.func_decl.name, params->count, args->count);
        return runtime_error(ctx, call, 3006, msg);
    }
    if (ctx->call_depth >= MAX_CALL_DEPTH) {
        return runtime_error(ctx, call, 4004,
                             "Demasiadas llamadas anidadas (¿recursión sin caso base?)");
    }

    /* Los argumentos se evalúan en el scope de quien llama */
    Value *values = NULL;
    if (args->count > 0) {
        values = malloc(args->count * sizeof(Value));
        if (!values) return runtime_error(ctx, call, 5002, NULL);
    }
    for (size_t i = 0; i < args->count; i++) {
        EvalResult a = interpret_node(ctx, args->data[i]);
        if (a.flow != FLOW_NORMAL) {
            free(values);
            return a;
        }
        values[i] = a.value;
    }

    /* La función ve las variables globales, no las locales de quien la llama */
    Scope *fn_scope = scope_new(ctx->global_scope);
    if (!fn_scope) {
        free(values);
        return runtime_error(ctx, call, 5002, NULL);
    }
    for (size_t i = 0; i < params->count; i++) {
        AstNode *param = params->data[i];
        const char *pname = interner_intern_cstr(ctx->interner, param->as.param.name);
        scope_define(fn_scope, pname, values[i]);
        scope_set_type(fn_scope, pname, param->as.param.type_name);
    }
    free(values);

    Scope *prev = ctx->current_scope;
    ctx->current_scope = fn_scope;
    ctx->call_depth++;
    EvalResult body = interpret_node(ctx, fn->as.func_decl.body);
    ctx->call_depth--;
    ctx->current_scope = prev;
    scope_destroy(fn_scope);

    EvalResult result = { .flow = FLOW_NORMAL, .value = value_null() };
    if (body.flow == FLOW_ERROR) return body;
    if (body.flow == FLOW_RETURN) result.value = body.value;
    return result;
}

/* size(), append() y remove() de listas: el parser los convierte en llamadas cuyo primer
   argumento es la lista */
static EvalResult call_list_method(InterpreterContext *ctx, AstNode *call, const char *method) {
    AstList *args = &call->as.call.args;
    if (args->count < 1) return runtime_error(ctx, call, 3006, NULL);

    EvalResult target = interpret_node(ctx, args->data[0]);
    if (target.flow != FLOW_NORMAL) return target;
    if (target.value.type != VAL_LIST || !target.value.as.list) {
        char msg[96];
        snprintf(msg, sizeof(msg), "%s() solo se puede usar con listas", method);
        return runtime_error(ctx, call, 4003, msg);
    }
    ListObj *l = target.value.as.list;
    EvalResult result = { .flow = FLOW_NORMAL, .value = value_null() };

    if (strcmp(method, "size") == 0) {
        result.value = value_integer((long)l->count);
        return result;
    }

    if (args->count < 2) return runtime_error(ctx, call, 3006, NULL);
    if (l->fixed) {
        return runtime_error(ctx, call, 4003,
                             "Un arreglo tiene tamaño fijo: usa List para agregar o quitar elementos");
    }
    EvalResult arg = interpret_node(ctx, args->data[1]);
    if (arg.flow != FLOW_NORMAL) return arg;

    if (strcmp(method, "append") == 0) {
        if (!list_push_value(l, arg.value)) return runtime_error(ctx, call, 5002, NULL);
        return result;
    }

    /* remove(índice) */
    if (arg.value.type != VAL_INTEGER) {
        return runtime_error(ctx, call, 4003, "remove() recibe la posición (Integer) a quitar");
    }
    long i = arg.value.as.integer;
    if (i < 0 || (size_t)i >= l->count) {
        char msg[96];
        snprintf(msg, sizeof(msg), "Índice %ld fuera de rango (la lista tiene %zu elementos)", i, l->count);
        return runtime_error(ctx, call, 4002, msg);
    }
    memmove(&l->items[i], &l->items[i + 1], (l->count - (size_t)i - 1) * sizeof(Value));
    l->count--;
    return result;
}

static EvalResult eval_call(InterpreterContext *ctx, AstNode *node) {
    AstNode *callee = node->as.call.callee;
    if (!callee || callee->kind != AST_IDENT) {
        return runtime_error(ctx, node, 4003, "Solo se pueden llamar funciones por su nombre");
    }
    const char *name = interner_intern_cstr(ctx->interner, callee->as.ident.lexeme);

    AstNode *fn = find_function(ctx, name);
    if (fn) return call_user_function(ctx, node, fn);

    if (strcmp(name, "size") == 0 || strcmp(name, "append") == 0 || strcmp(name, "remove") == 0) {
        return call_list_method(ctx, node, name);
    }

    DiagLocation loc = diag_location_create(node->span.line_start, node->span.col_start,
                                            strlen(name), name);
    diag_error(STAGE_SEMANTIC, 3005, loc, NULL);
    ctx->has_error = true;
    EvalResult r = { .flow = FLOW_ERROR, .value = value_null() };
    return r;
}

/* ============ Indexación ============ */

/* Deja en *slot la posición del elemento base[índice] de un arreglo o lista */
static EvalResult resolve_index(InterpreterContext *ctx, AstNode *node, Value **slot) {
    EvalResult base = interpret_node(ctx, node->as.index.base);
    if (base.flow != FLOW_NORMAL) return base;
    EvalResult idx = interpret_node(ctx, node->as.index.index);
    if (idx.flow != FLOW_NORMAL) return idx;

    if (idx.value.type != VAL_INTEGER) {
        return runtime_error(ctx, node, 4003, "El índice debe ser un Integer");
    }
    if (base.value.type != VAL_LIST || !base.value.as.list) {
        return runtime_error(ctx, node, 4003, "Solo se pueden indexar arreglos y listas");
    }
    ListObj *l = base.value.as.list;
    long i = idx.value.as.integer;
    if (i < 0 || (size_t)i >= l->count) {
        char msg[96];
        snprintf(msg, sizeof(msg), "Índice %ld fuera de rango (tiene %zu elementos)", i, l->count);
        return runtime_error(ctx, node, 4002, msg);
    }
    *slot = &l->items[i];
    EvalResult ok = { .flow = FLOW_NORMAL, .value = **slot };
    return ok;
}

static EvalResult eval_index(InterpreterContext *ctx, AstNode *node) {
    /* Texto[i] devuelve el Char en esa posición */
    EvalResult base = interpret_node(ctx, node->as.index.base);
    if (base.flow != FLOW_NORMAL) return base;
    if (base.value.type == VAL_STRING) {
        EvalResult idx = interpret_node(ctx, node->as.index.index);
        if (idx.flow != FLOW_NORMAL) return idx;
        if (idx.value.type != VAL_INTEGER) {
            return runtime_error(ctx, node, 4003, "El índice debe ser un Integer");
        }
        size_t n;
        const char *c = string_content(base.value.as.string, &n);
        long i = idx.value.as.integer;
        if (i < 0 || (size_t)i >= n) {
            char msg[96];
            snprintf(msg, sizeof(msg), "Índice %ld fuera de rango (el texto tiene %zu caracteres)", i, n);
            return runtime_error(ctx, node, 4002, msg);
        }
        EvalResult r = { .flow = FLOW_NORMAL, .value = value_char(c[i]) };
        return r;
    }
    Value *slot = NULL;
    return resolve_index(ctx, node, &slot);
}

/* ============ Declaración de arreglos y listas ============ */

static EvalResult eval_collection_decl(InterpreterContext *ctx, AstNode *node, const char *type_name,
                                       bool is_array) {
    const char *element_type = type_name + (is_array ? 6 : 5); /* tras "Array:" / "List:" */
    AstNode *init = node->as.var_decl.init;
    AstList *elements = NULL;
    size_t size = 0;

    if (is_array) {
        /* init = BLOQUE [tamaño, BLOQUE de elementos] */
        if (!init || init->kind != AST_BLOCK || init->as.block.stmts.count < 1) {
            return runtime_error(ctx, node, 4003, "Falta el tamaño del arreglo");
        }
        EvalResult sz = interpret_node(ctx, init->as.block.stmts.data[0]);
        if (sz.flow != FLOW_NORMAL) return sz;
        if (sz.value.type != VAL_INTEGER || sz.value.as.integer < 0) {
            return runtime_error(ctx, node, 4003, "El tamaño del arreglo debe ser un Integer no negativo");
        }
        if (sz.value.as.integer > MAX_ARRAY_SIZE) {
            return runtime_error(ctx, node, 4003, "El arreglo es demasiado grande (máximo 1000000)");
        }
        size = (size_t)sz.value.as.integer;
        if (init->as.block.stmts.count >= 2 && init->as.block.stmts.data[1]) {
            elements = &init->as.block.stmts.data[1]->as.block.stmts;
        }
        if (elements && elements->count > size) {
            char msg[128];
            snprintf(msg, sizeof(msg), "El arreglo tiene tamaño %zu pero se le dieron %zu valores",
                     size, elements->count);
            return runtime_error(ctx, node, 4002, msg);
        }
    } else if (init && init->kind == AST_BLOCK) {
        elements = &init->as.block.stmts;
    }

    size_t count = elements ? elements->count : 0;
    ListObj *l = list_new(ctx, is_array ? size : (count > 4 ? count : 4), is_array);
    if (!l) return runtime_error(ctx, node, 5002, NULL);

    if (is_array) {
        for (size_t i = 0; i < size; i++) l->items[i] = default_value_for(element_type);
        l->count = size;
    }
    for (size_t i = 0; i < count; i++) {
        EvalResult e = interpret_node(ctx, elements->data[i]);
        if (e.flow != FLOW_NORMAL) return e;
        if (is_array) {
            l->items[i] = e.value;
        } else if (!list_push_value(l, e.value)) {
            return runtime_error(ctx, node, 5002, NULL);
        }
    }

    const char *name = interner_intern_cstr(ctx->interner, node->as.var_decl.name);
    scope_define(ctx->current_scope, name, value_list(l));
    EvalResult r = { .flow = FLOW_NORMAL, .value = value_list(l) };
    return r;
}

/* ============ Read ============ */

static EvalResult store_read_value(InterpreterContext *ctx, AstNode *target, Value *slot,
                                   const char *text) {
    char *end = NULL;
    switch (slot->type) {
        case VAL_INTEGER: {
            long v = strtol(text, &end, 10);
            if (end == text || *end != '\0') break;
            *slot = value_integer(v);
            return (EvalResult){ .flow = FLOW_NORMAL, .value = *slot };
        }
        case VAL_FLOAT: {
            double v = strtod(text, &end);
            if (end == text || *end != '\0') break;
            *slot = value_float(v);
            return (EvalResult){ .flow = FLOW_NORMAL, .value = *slot };
        }
        case VAL_BOOLEAN:
            if (strcmp(text, "True") == 0 || strcmp(text, "true") == 0) {
                *slot = value_boolean(true);
            } else if (strcmp(text, "False") == 0 || strcmp(text, "false") == 0) {
                *slot = value_boolean(false);
            } else {
                break;
            }
            return (EvalResult){ .flow = FLOW_NORMAL, .value = *slot };
        case VAL_CHAR:
            if (strlen(text) != 1) break;
            *slot = value_char(text[0]);
            return (EvalResult){ .flow = FLOW_NORMAL, .value = *slot };
        case VAL_STRING:
        case VAL_NULL: {
            /* Se guarda en la arena del AST: vive hasta el final del programa */
            char *copy = ast_arena_strndup(ctx->arena, text, strlen(text));
            if (!copy) return runtime_error(ctx, target, 5002, NULL);
            *slot = value_string(copy);
            return (EvalResult){ .flow = FLOW_NORMAL, .value = *slot };
        }
        default:
            return runtime_error(ctx, target, 4003, "Read no puede leer arreglos ni listas");
    }
    char msg[MAX_READ_LINE + 64];
    snprintf(msg, sizeof(msg), "Se esperaba un %s y se leyó \"%s\"", value_type_name(slot->type), text);
    return runtime_error(ctx, target, 4003, msg);
}

static EvalResult eval_read(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL, .value = value_null() };
    for (size_t i = 0; i < node->as.read.targets.count; i++) {
        AstNode *target = node->as.read.targets.data[i];
        const char *name = interner_intern_cstr(ctx->interner, target->as.ident.lexeme);
        Value *slot = scope_lookup(ctx->current_scope, name);
        if (!slot) {
            DiagLocation loc = diag_location_create(target->span.line_start, target->span.col_start,
                                                    strlen(name), name);
            diag_error(STAGE_SEMANTIC, 3001, loc, NULL);
            ctx->has_error = true;
            result.flow = FLOW_ERROR;
            return result;
        }

        char line[MAX_READ_LINE];
        if (!fgets(line, sizeof(line), stdin)) {
            return runtime_error(ctx, target, 4006, "No hay más datos de entrada para Read");
        }
        line[strcspn(line, "\r\n")] = '\0';

        if (slot->type == VAL_NULL) {
            /* Sin valor todavía: tomar el tipo declarado (0 / 0.0 / False / "" solo indican el tipo) */
            Value typed = default_value_for(scope_lookup_type(ctx->current_scope, name));
            if (typed.type != VAL_NULL) *slot = typed;
        }
        result = store_read_value(ctx, target, slot, line);
        if (result.flow != FLOW_NORMAL) return result;
    }
    return result;
}

EvalResult interpret_node(InterpreterContext *ctx, AstNode *node) {
    if (!node) {
        EvalResult result = { .flow = FLOW_NORMAL, .value = value_null() };
        return result;
    }
    
    switch (node->kind) {
        case AST_LITERAL:
            return eval_literal(ctx, node);
        case AST_IDENT:
            return eval_identifier(ctx, node);
        case AST_BINOP:
            return eval_binary_op(ctx, node);
        case AST_UNOP:
            return eval_unary_op(ctx, node);
        case AST_ASSIGN:
            return eval_assignment(ctx, node);
        case AST_VAR_DECL:
            return eval_var_decl(ctx, node);
        case AST_PRINT:
            return eval_print(ctx, node);
        case AST_IF:
            return eval_if(ctx, node);
        case AST_WHILE:
            return eval_while(ctx, node);
        case AST_FOR:
            return eval_for(ctx, node);
        case AST_BLOCK:
            return eval_block(ctx, node);
        case AST_RETURN:
            return eval_return(ctx, node);
        case AST_EXPR:
            return interpret_node(ctx, node->as.expr.expr);
        
        case AST_FUNC_DECL:
            return eval_func_decl(ctx, node);
        case AST_CALL:
            return eval_call(ctx, node);
        case AST_INDEX:
            return eval_index(ctx, node);
        case AST_READ:
            return eval_read(ctx, node);
        case AST_PARAM:
        case AST_ERROR:
        default: {
            EvalResult result = { .flow = FLOW_NORMAL, .value = value_null() };
            return result;
        }
    }
}

EvalResult interpret_program(InterpreterContext *ctx, AstNode *program) {
    if (!program || program->kind != AST_PROGRAM) {
        EvalResult result = { .flow = FLOW_ERROR, .value = value_null() };
        return result;
    }
    
    EvalResult result = { .flow = FLOW_NORMAL, .value = value_null() };
    
    // Ejecutar todas las declaraciones del programa
    for (size_t i = 0; i < program->as.program.decls.count; i++) {
        EvalResult decl_result = interpret_node(ctx, program->as.program.decls.data[i]);
        
        if (decl_result.flow == FLOW_ERROR) {
            return decl_result;
        }
        if (decl_result.flow == FLOW_RETURN) {
            // Return en nivel global termina el programa
            result.value = decl_result.value;
            break;
        }
    }
    
    return result;
}