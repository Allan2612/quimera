#include "interpreter.h"
#include "symbol_table.h"
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
            return val->as.list.count > 0;
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
            return strcmp(a->as.string, b->as.string) == 0;
        case VAL_BOOLEAN:
            return a->as.boolean == b->as.boolean;
        case VAL_CHAR:
            return a->as.char_val == b->as.char_val;
        case VAL_LIST:
            // Comparación básica por referencia por ahora
            return &a->as.list == &b->as.list;
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
        case VAL_STRING:
            return strcmp(a->as.string, b->as.string);
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
        ErrorToken tok = { 4001, NULL, 0, span.line_start, span.col_start };
        diag_report(ST_RUNTIME, SV_WARNING, tok, "División por cero");
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
        ErrorToken tok = { 4001, NULL, 0, span.line_start, span.col_start };
        diag_report(ST_RUNTIME, SV_WARNING, tok, "Módulo por cero");
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

void value_print(const Value *val) {
    switch (val->type) {
        case VAL_NULL:
            printf("Null");
            break;
        case VAL_INTEGER:
            printf("%ld", val->as.integer);
            break;
        case VAL_FLOAT:
            printf("%g", val->as.float_val);
            break;
        case VAL_STRING: {
            const char *str = val->as.string;
            size_t len = strlen(str);
            // Si la string tiene comillas, quitarlas para la impresión
            if (len >= 2 && str[0] == '"' && str[len-1] == '"') {
                // Imprimir sin las comillas
                for (size_t i = 1; i < len - 1; i++) {
                    putchar(str[i]);
                }
            } else {
                printf("%s", str);
            }
            break;
        }
        case VAL_BOOLEAN:
            printf("%s", val->as.boolean ? "True" : "False");
            break;
        case VAL_CHAR:
            printf("%c", val->as.char_val);
            break;
        case VAL_LIST:
            printf("[List with %zu elements]", val->as.list.count);
            break;
        default:
            printf("<unknown>");
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
    
    if (!scope->names || !scope->values) {
        free(scope->names);
        free(scope->values);
        free(scope);
        return NULL;
    }
    
    return scope;
}

void scope_destroy(Scope *scope) {
    if (!scope) return;
    free(scope->names);
    free(scope->values);
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
        Value *new_values = realloc(scope->values, new_cap * sizeof(Value));
        
        if (!new_names || !new_values) return;
        
        scope->names = new_names;
        scope->values = new_values;
        scope->capacity = new_cap;
    }

    scope->names[scope->count] = name;
    scope->values[scope->count] = value;
    scope->count++;
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
}

/* ============ Evaluación de nodos ============ */

static EvalResult eval_literal(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL };
    
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
    }
    
    return result;
}

static EvalResult eval_identifier(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL };
    
    const char *name = node->as.ident.lexeme;
    const char *interned_name = interner_intern_cstr(ctx->interner, name);
    
    Value *val = scope_lookup(ctx->current_scope, interned_name);
    if (!val) {
        ErrorToken tok = { 3001, name, strlen(name), node->span.line_start, node->span.col_start };
        diag_report(ST_SEMANTIC, SV_ERROR, tok, NULL);
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
            result.value = value_divide(left, right, node->span);
            break;
        case OP_MOD:
            result.value = value_modulo(left, right, node->span);
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
                ErrorToken tok = { 3002, NULL, 0, node->span.line_start, node->span.col_start };
                diag_report(ST_SEMANTIC, SV_WARNING, tok, "Comparación entre tipos incompatibles");
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
    
    // Solo soportamos asignación a identificadores por ahora
    if (node->as.assign.lhs->kind != AST_IDENT) {
        result.value = value_null();
        result.flow = FLOW_ERROR;
        return result;
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
            ErrorToken tok = { 3001, name, strlen(name), node->span.line_start, node->span.col_start };
            diag_report(ST_SEMANTIC, SV_ERROR, tok, NULL);
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
        ErrorToken tok = { 3001, name, strlen(name), node->span.line_start, node->span.col_start };
        diag_report(ST_SEMANTIC, SV_ERROR, tok, NULL);
        result.flow = FLOW_ERROR;
    } else {
        result.value = final_value;
    }
    
    return result;
}

static EvalResult eval_var_decl(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL };
    
    const char *name = interner_intern_cstr(ctx->interner, node->as.var_decl.name);
    Value init_value = value_null();
    
    if (node->as.var_decl.init) {
        EvalResult init_result = interpret_node(ctx, node->as.var_decl.init);
        if (init_result.flow != FLOW_NORMAL) return init_result;
        init_value = init_result.value;
    }
    
    scope_define(ctx->current_scope, name, init_value);
    result.value = init_value;
    
    return result;
}

static EvalResult eval_print(InterpreterContext *ctx, AstNode *node) {
    EvalResult result = { .flow = FLOW_NORMAL };
    
    for (size_t i = 0; i < node->as.print.exprs.count; i++) {
        if (i > 0) printf(" ");
        
        EvalResult expr_result = interpret_node(ctx, node->as.print.exprs.data[i]);
        if (expr_result.flow != FLOW_NORMAL) return expr_result;
        
        value_print(&expr_result.value);
    }
    printf("\n");
    
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
        case AST_BLOCK:
            return eval_block(ctx, node);
        case AST_RETURN:
            return eval_return(ctx, node);
        case AST_EXPR:
            return interpret_node(ctx, node->as.expr.expr);
        
        // TODO: Implementar casos faltantes
        case AST_FUNC_DECL:
        case AST_PARAM:
        case AST_FOR:
        case AST_READ:
        case AST_CALL:
        case AST_INDEX:
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