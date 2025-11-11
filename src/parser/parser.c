#include "parser.h"
#include "symbol_table.h"
#include "diag_errors.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ========== util de tokens ========== */

static inline int is_eof(Parser *p) { return p->i >= p->count; }
static inline const Token *at(Parser *p) { return is_eof(p) ? NULL : &p->toks[p->i]; }
static inline const Token *peek(Parser *p, size_t k) { return (p->i + k >= p->count) ? NULL : &p->toks[p->i + k]; }
static inline const Token *advance(Parser *p) { return is_eof(p) ? NULL : &p->toks[p->i++]; }

static inline int tok_is(const Token *t, TokenType ty) { return t && t->type == ty; }

/* Helper: Conversión de token a string internado */
static const char *intern_token_lexeme(Parser *p, const Token *t) {
    if (!t || !t->lexeme) return NULL;
    return interner_intern_cstr(p->interner, t->lexeme);
}

/* Helper: Comparación de lexema con string internado - VERSIÓN ÚNICA */
static int lexeme_is_intern(Parser *p, const Token *t, const char *interned_str) {
    if (!t || !t->lexeme || !interned_str) return 0;
    const char *interned_lexeme = intern_token_lexeme(p, t);
    return interned_lexeme == interned_str;
}

/* Helper: Funciones para obtener strings internados comunes */
static const char *get_intern_delim(Parser *p, const char *delim) {
    return interner_intern_cstr(p->interner, delim);
}

static const char *get_intern_op(Parser *p, const char *op) {
    return interner_intern_cstr(p->interner, op);
}

static const char *get_intern_kw(Parser *p, const char *kw) {
    return interner_intern_cstr(p->interner, kw);
}

/* Helper para compatibilidad temporal */
static int lexeme_is(const Token *t, const char *s) {
    if (!t || !t->lexeme || !s) return 0;
    return strncmp(t->lexeme, s, t->length) == 0 && strlen(s) == t->length;
}

/* Token check functions - SOLO INTERNER */
static int is_kw_intern(Parser *p, const Token *t, const char *interned_kw) {
    return tok_is(t, TOK_KEYWORD) && lexeme_is_intern(p, t, interned_kw);
}

/* Funciones temporales de compatibilidad - TODO: migrar a _intern */
static int is_kw(const Token *t, const char *kw) {
    return tok_is(t, TOK_KEYWORD) && lexeme_is(t, kw);
}

static int is_delim(const Token *t, const char *d) {
    return tok_is(t, TOK_DELIMITER) && lexeme_is(t, d);
}

static int is_type(const Token *t) {
    if (!tok_is(t, TOK_KEYWORD)) return 0;
    for (size_t i = 0; i < BuiltinTypeCount; i++) {
        if (lexeme_is(t, BuiltinTypes[i])) return 1;
    }
    return 0;
}

static int is_delim_intern(Parser *p, const Token *t, const char *interned_delim) {
    return tok_is(t, TOK_DELIMITER) && lexeme_is_intern(p, t, interned_delim);
}

static int is_assignment_intern(Parser *p, const Token *t, const char *interned_op) {
    return tok_is(t, TOK_ASSIGNMENT_OP) && lexeme_is_intern(p, t, interned_op);
}

static int is_arithmetic_intern(Parser *p, const Token *t, const char *interned_op) {
    return tok_is(t, TOK_ARITHMETIC_OP) && lexeme_is_intern(p, t, interned_op);
}

static int is_logic_intern(Parser *p, const Token *t, const char *interned_op) {
    return tok_is(t, TOK_LOGIC_OP) && lexeme_is_intern(p, t, interned_op);
}

static int is_type_intern(Parser *p, const Token *t) {
    if (!tok_is(t, TOK_KEYWORD)) return 0;
    const char *t_interned = intern_token_lexeme(p, t);
    return (t_interned == p->syms->T_Integer ||
            t_interned == p->syms->T_Float ||
            t_interned == p->syms->T_String ||
            t_interned == p->syms->T_Boolean ||
            t_interned == p->syms->T_Char);
}

static SrcSpan span_of_tok(const Token *t) {
    SrcSpan s = {0,0,0,0};
    if (t) {
        s.line_start = s.line_end = (int)t->line;
        s.col_start  = (int)t->column;
        s.col_end    = (int)(t->column + t->length);
    }
    return s;
}

static SrcSpan span_range(const Token *start, const Token *end) {
    SrcSpan s = {0,0,0,0};
    if (start) {
        s.line_start = (int)start->line;
        s.col_start = (int)start->column;
    }
    if (end) {
        s.line_end = (int)end->line;
        s.col_end = (int)(end->column + end->length);
    } else if (start) {
        s.line_end = (int)start->line;
        s.col_end = (int)(start->column + start->length);
    }
    return s;
}

/* Diagnóstico integrado con diag_errors */
static void err(Parser *p, const Token *t, const char *msg) {
    ++p->error_count;
    if (t) {
        ErrorToken et = {
            .code = 2001, // Error genérico de parser
            .lexeme = t->lexeme,
            .length = t->length,
            .line = t->line,
            .column = t->column
        };
        diag_report(ST_PARSER, SV_ERROR, et, msg);
    } else {
        ErrorToken et = {
            .code = 2002,
            .lexeme = "",
            .length = 0,
            .line = 0,
            .column = 0
        };
        diag_report(ST_PARSER, SV_ERROR, et, msg);
    }
}

/* Consumo con expectativa básica */
static const Token *expect_kw(Parser *p, const char *kw) {
    const Token *t = at(p);
    if (is_kw(t, kw)) return advance(p);
    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg), "Se esperaba palabra clave '%s'", kw);
    err(p, t, error_msg);
    return t;
}

/* Versión optimizada para palabras clave internadas */
static const Token *expect_kw_intern(Parser *p, const char *interned_kw) {
    const Token *t = at(p);
    if (is_kw_intern(p, t, interned_kw)) return advance(p);
    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg), "Se esperaba palabra clave internada");
    err(p, t, error_msg);
    return t;
}

static const Token *expect_delim(Parser *p, const char *d) {
    const Token *t = at(p);
    if (is_delim(t, d)) return advance(p);
    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg), "Se esperaba delimitador '%s'", d);
    err(p, t, error_msg);
    return t;
}

/* Versión optimizada para delimitadores internados */
static const Token *expect_delim_intern(Parser *p, const char *interned_delim) {
    const Token *t = at(p);
    if (is_delim_intern(p, t, interned_delim)) return advance(p);
    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg), "Se esperaba delimitador internado");
    err(p, t, error_msg);
    return t;
}

static const Token *expect_type(Parser *p) {
    const Token *t = at(p);
    if (is_type_intern(p, t)) return advance(p);
    err(p, t, "Se esperaba tipo (String, Integer, Float, Boolean, Char)");
    return t;
}

static const Token *expect_identifier(Parser *p) {
    const Token *t = at(p);
    if (tok_is(t, TOK_IDENTIFIER)) return advance(p);
    err(p, t, "Se esperaba identificador");
    return t;
}

/* ========== AST helpers ========== */

static AstNode *make_node(Parser *p, AstKind k, const Token *t) {
    return ast_new(p->arena, k, span_of_tok(t));
}

static AstNode *make_node_span(Parser *p, AstKind k, const Token *start, const Token *end) {
    AstNode *n = ast_new(p->arena, k, span_range(start, end));
    return n;
}

static void list_push(Parser *p, AstList *lst, AstNode *n) {
    ast_list_push(p->arena, lst, n);
}

/* ========== Funciones forward ========== */
static AstNode *parse_declaration(Parser *p);
static AstNode *parse_var_declaration(Parser *p);
static AstNode *parse_array_declaration(Parser *p);
static AstNode *parse_list_declaration(Parser *p);
static AstNode *parse_function_declaration(Parser *p);
static AstNode *parse_for_stmt(Parser *p);
static AstNode *parse_statement(Parser *p);
static AstNode *parse_expression(Parser *p);

/* ========== Tabla de precedencias (Pratt) ==========

   Mayor → menor:
   8: llamados/index   (posfijos) func(), arr[i], list.size()
   7: **               (exponenciación)
   6: unarios          (!, -, +)
   5: *, /, %
   4: +, -
   3: comparadores     (<, <=, >, >=)
   2: igualdad         (==, !=)
   1: lógicos          (And, Or)
   0: asignación       (=, +=, -=)  (asociatividad derecha)
*/

typedef enum {
    PREC_ASSIGN = 0,
    PREC_LOGIC  = 1,
    PREC_EQUAL  = 2,
    PREC_CMP    = 3,
    PREC_ADD    = 4,
    PREC_MUL    = 5,
    PREC_UNARY  = 6,
    PREC_POW    = 7,
    PREC_POST   = 8
} Precedence;

static Precedence infix_prec_from_tok(const Token *t) {
    if (!t) return PREC_ASSIGN;
    if (tok_is(t, TOK_ASSIGNMENT_OP)) return PREC_ASSIGN;
    
    if (tok_is(t, TOK_ARITHMETIC_OP)) {
        if (lexeme_is(t, "**")) return PREC_POW;
        if (lexeme_is(t, "*") || lexeme_is(t, "/") || lexeme_is(t, "%")) return PREC_MUL;
        if (lexeme_is(t, "+") || lexeme_is(t, "-")) return PREC_ADD;
    }
    
    if (tok_is(t, TOK_LOGIC_OP)) {
        if (lexeme_is(t, "And") || lexeme_is(t, "Or")) return PREC_LOGIC;
        if (lexeme_is(t, "==") || lexeme_is(t, "!=")) return PREC_EQUAL;
        if (lexeme_is(t, "<") || lexeme_is(t, "<=") || lexeme_is(t, ">") || lexeme_is(t, ">=")) return PREC_CMP;
    }
    
    if (tok_is(t, TOK_DELIMITER) && (lexeme_is(t, "(") || lexeme_is(t, "["))) return PREC_POST;
    
    return PREC_ASSIGN;
}

static OpCode opcode_from_tok(const Token *t) {
    if (!t) return OP_NONE;
    
    /* asignaciones */
    if (tok_is(t, TOK_ASSIGNMENT_OP)) {
        if (lexeme_is(t,"="))  return OP_ASSIGN;
        if (lexeme_is(t,"+=")) return OP_ADD_ASSIGN;
        if (lexeme_is(t,"-=")) return OP_SUB_ASSIGN;
    }
    
    /* aritméticos */
    if (tok_is(t, TOK_ARITHMETIC_OP)) {
        if (lexeme_is(t,"+")) return OP_ADD;
        if (lexeme_is(t,"-")) return OP_SUB;
        if (lexeme_is(t,"*")) return OP_MUL;
        if (lexeme_is(t,"/")) return OP_DIV;
        if (lexeme_is(t,"%")) return OP_MOD;
    }
    
    /* lógicos / comparadores */
    if (tok_is(t, TOK_LOGIC_OP)) {
        if (lexeme_is(t,"And")) return OP_AND;
        if (lexeme_is(t,"Or"))  return OP_OR;
        if (lexeme_is(t,"Not"))  return OP_NOT;
        if (lexeme_is(t,"=="))  return OP_EQ;
        if (lexeme_is(t,"!="))  return OP_NEQ;
        if (lexeme_is(t,"<"))   return OP_LT;
        if (lexeme_is(t,"<="))  return OP_LTE;
        if (lexeme_is(t,">"))   return OP_GT;
        if (lexeme_is(t,">="))  return OP_GTE;
    }
    
    return OP_NONE;
}

/* Función específica para operadores unarios */
static OpCode opcode_from_tok_unary(const Token *t) {
    if (!t) return OP_NONE;
    
    if (tok_is(t, TOK_LOGIC_OP)){
        if (lexeme_is(t,"Not"))  return OP_NOT;
        if (lexeme_is(t,"!"))    return OP_NOT;
    }
    
    if (tok_is(t, TOK_ARITHMETIC_OP)) {
        if (lexeme_is(t,"-")) return OP_NEG;
        if (lexeme_is(t,"+")) return OP_ADD; // + unario (normalmente no hace nada)
    }
    
    return OP_NONE;
}

/* Versión optimizada que usa interner */
static OpCode opcode_from_tok_intern(Parser *p, const Token *t) {
    if (!t) return OP_NONE;
    
    /* asignaciones */
    if (tok_is(t, TOK_ASSIGNMENT_OP)) {
        if (lexeme_is(t,"="))  return OP_ASSIGN;
        if (lexeme_is(t,"+=")) return OP_ADD_ASSIGN;
        if (lexeme_is(t,"-=")) return OP_SUB_ASSIGN;
    }
    
    /* aritméticos */
    if (tok_is(t, TOK_ARITHMETIC_OP)) {
        if (lexeme_is(t,"+")) return OP_ADD;
        if (lexeme_is(t,"-")) return OP_SUB;
        if (lexeme_is(t,"*")) return OP_MUL;
        if (lexeme_is(t,"/")) return OP_DIV;
        if (lexeme_is(t,"%")) return OP_MOD;
    }
    
    /* lógicos / comparadores */
    if (tok_is(t, TOK_LOGIC_OP)) {
        if (lexeme_is(t,"And")) return OP_AND;
        if (lexeme_is(t,"Or"))  return OP_OR;
        if (lexeme_is(t,"=="))  return OP_EQ;
        if (lexeme_is(t,"!="))  return OP_NEQ;
        if (lexeme_is(t,"<"))   return OP_LT;
        if (lexeme_is(t,"<="))  return OP_LTE;
        if (lexeme_is(t,">"))   return OP_GT;
        if (lexeme_is(t,">="))  return OP_GTE;
    }
    
    /* unarios por keyword u op */
    if (is_kw_intern(p, t, p->syms->K_Not)) return OP_NOT;
    if (tok_is(t, TOK_ARITHMETIC_OP) && lexeme_is(t,"-")) return OP_NEG;
    
    return OP_NONE;
}

/* ========== Forward de expresiones (Pratt) ========== */
static AstNode *parse_precedence(Parser *p, int min_prec);

/* Funciones auxiliares para parsear listas */
static AstNode *parse_list_elements(Parser *p) {
    AstNode *list = make_node(p, AST_BLOCK, at(p)); // Usar AST_BLOCK como contenedor temporal
    expect_delim(p, "{");
    
    if (!is_delim(at(p), "}")) {
        do {
            AstNode *expr = parse_expression(p);
            if (expr) list_push(p, &list->as.block.stmts, expr);
        } while (is_delim(at(p), ",") && advance(p));
    }
    
    expect_delim(p, "}");
    return list;
}

/* Prefijos: ident, literal, (expr), Not, -, + */
static AstNode *parse_prefix(Parser *p) {
    const Token *t = at(p);
    if (!t) { 
        err(p, t, "Expresión incompleta"); 
        return ast_new(p->arena, AST_ERROR, span_of_tok(t)); 
    }

    /* paréntesis */
    if (is_delim(t, "(")) {
        advance(p);
        AstNode *e = parse_expression(p);
        expect_delim(p, ")");
        return e ? e : ast_new(p->arena, AST_ERROR, span_of_tok(t));
    }

    /* unarios: Not, -, + */
    if ((tok_is(t, TOK_LOGIC_OP) && (lexeme_is(t,"Not") || lexeme_is(t,"!"))) || (tok_is(t, TOK_ARITHMETIC_OP) && (lexeme_is(t,"-") || lexeme_is(t,"+")))) {
        OpCode op = opcode_from_tok_unary(t);
        advance(p);
        AstNode *rhs = parse_precedence(p, PREC_UNARY);
        AstNode *u = make_node(p, AST_UNOP, t);
        u->as.unop.op = op;
        u->as.unop.expr = rhs;
        return u;
    }

    /* identificador */
    if (tok_is(t, TOK_IDENTIFIER)) {
        AstNode *id = make_node(p, AST_IDENT, t);
        id->as.ident.lexeme = intern_token_lexeme(p, t); // String internada
        advance(p);
        return id;
    }

    /* literales */
    if (tok_is(t, TOK_INTEGER) || tok_is(t, TOK_FLOAT) || tok_is(t, TOK_STRING) || 
        tok_is(t, TOK_CHAR) || tok_is(t, TOK_BOOLEAN) || is_kw(t,"Null") ||
        is_kw(t,"True") || is_kw(t,"False")) {
        
        AstNode *lit = make_node(p, AST_LITERAL, t);
        
        if (tok_is(t, TOK_INTEGER)) { 
            lit->as.literal.tag = LIT_INT;
            lit->as.literal.val.i = strtol(t->lexeme, NULL, 10);
        }
        else if (tok_is(t, TOK_FLOAT)) { 
            lit->as.literal.tag = LIT_FLOAT; 
            lit->as.literal.val.f = strtod(t->lexeme, NULL);
        }
        else if (tok_is(t, TOK_STRING) || tok_is(t, TOK_CHAR)) { 
            lit->as.literal.tag = LIT_STRING; 
            lit->as.literal.val.s = intern_token_lexeme(p, t); // String internada
        }
        else if (tok_is(t, TOK_BOOLEAN) || is_kw_intern(p, t, p->syms->K_True) || is_kw_intern(p, t, p->syms->K_False)) { 
            lit->as.literal.tag = LIT_BOOL;
            const char *t_interned = intern_token_lexeme(p, t);
            lit->as.literal.val.b = (t_interned == p->syms->K_True);
        }
        else if (is_kw_intern(p, t, p->syms->K_Null)) { 
            lit->as.literal.tag = LIT_NULL; 
        }
        
        advance(p);
        return lit;
    }

    err(p, t, "Token no válido al inicio de expresión");
    advance(p);
    return ast_new(p->arena, AST_ERROR, span_of_tok(t));
}

/* Posfijos: llamadas, indexación y métodos de lista */
static AstNode *parse_postfix(Parser *p, AstNode *lhs) {
    for (;;) {
        const Token *t = at(p);
        if (!t) break;

        /* llamada: func(arglist) */
        if (is_delim(t, "(")) {
            advance(p); // '('
            AstNode *call = make_node(p, AST_CALL, t);
            call->as.call.callee = lhs;
            AstList *args = &call->as.call.args;

            if (!is_delim(at(p), ")")) {
                do {
                    AstNode *arg = parse_expression(p);
                    if (arg) list_push(p, args, arg);
                } while (is_delim(at(p), ",") && advance(p));
            }
            expect_delim(p, ")");
            lhs = call;
            continue;
        }

        /* indexación: arr[expr] */
        if (is_delim(t, "[")) {
            advance(p); // '['
            AstNode *idx = make_node(p, AST_INDEX, t);
            idx->as.index.base = lhs;
            idx->as.index.index = parse_expression(p);
            expect_delim(p, "]");
            lhs = idx;
            continue;
        }

        /* métodos de lista: .append(), .remove(), .size() */
        if (tok_is(t, TOK_ARITHMETIC_OP) && lexeme_is(t, ".")) {
            const Token *dot = advance(p); // '.'
            const Token *method_name = at(p);
            
            if (tok_is(method_name, TOK_IDENTIFIER)) {
                if (lexeme_is(method_name, "append") || lexeme_is(method_name, "remove")) {
                    advance(p); // nombre del método
                    expect_delim(p, "(");
                    
                    AstNode *call = make_node(p, AST_CALL, dot);
                    // Crear identificador especial para el método
                    AstNode *method_id = make_node(p, AST_IDENT, method_name);
                    method_id->as.ident.lexeme = intern_token_lexeme(p, method_name); // String internada
                    call->as.call.callee = method_id;
                    
                    // Primer argumento es el objeto base
                    list_push(p, &call->as.call.args, lhs);
                    
                    // Segundo argumento es el valor
                    if (!is_delim(at(p), ")")) {
                        AstNode *arg = parse_expression(p);
                        if (arg) list_push(p, &call->as.call.args, arg);
                    }
                    
                    expect_delim(p, ")");
                    lhs = call;
                    continue;
                }
                else if (lexeme_is(method_name, "size")) {
                    advance(p); // nombre del método
                    expect_delim(p, "(");
                    expect_delim(p, ")");
                    
                    AstNode *call = make_node(p, AST_CALL, dot);
                    AstNode *method_id = make_node(p, AST_IDENT, method_name);
                    method_id->as.ident.lexeme = intern_token_lexeme(p, method_name); // String internada
                    call->as.call.callee = method_id;
                    list_push(p, &call->as.call.args, lhs);
                    
                    lhs = call;
                    continue;
                }
            }
            
            err(p, method_name, "Método de lista no válido");
            break;
        }

        break;
    }
    return lhs;
}

/* Infix/assign con precedencias (Pratt) */
static AstNode *parse_precedence(Parser *p, int min_prec) {
    AstNode *lhs = parse_prefix(p);
    if (!lhs) return NULL;
    
    lhs = parse_postfix(p, lhs);

    for (;;) {
        const Token *op_t = at(p);
        if (!op_t) break;

        Precedence prec = infix_prec_from_tok(op_t);
        if (prec < min_prec) break;

        /* asignación (asociativa a derecha) */
        if (tok_is(op_t, TOK_ASSIGNMENT_OP)) {
            OpCode op = opcode_from_tok(op_t);
            advance(p);
            AstNode *rhs = parse_precedence(p, PREC_ASSIGN); // right-assoc
            AstNode *as = make_node(p, AST_ASSIGN, op_t);
            as->as.assign.op = op;
            as->as.assign.lhs = lhs;
            as->as.assign.rhs = rhs;
            lhs = as;
            continue;
        }

        /* exponenciación ** (asociativa a derecha) */
        if (tok_is(op_t, TOK_ARITHMETIC_OP) && lexeme_is(op_t, "**")) {
            advance(p);
            AstNode *rhs = parse_precedence(p, PREC_POW); // right-assoc
            AstNode *bin = make_node(p, AST_BINOP, op_t);
            bin->as.binop.op = OP_MUL; // usando OP_MUL temporalmente para **
            bin->as.binop.left = lhs;
            bin->as.binop.right = rhs;
            lhs = bin;
            continue;
        }

        /* binarios normales (left-assoc) */
        OpCode op = opcode_from_tok(op_t);
        if (op == OP_NONE) break;
        
        int next_min = prec + 1; // left-assoc por defecto
        advance(p);

        AstNode *rhs = parse_precedence(p, next_min);
        AstNode *bin = make_node(p, AST_BINOP, op_t);
        bin->as.binop.op = op;
        bin->as.binop.left = lhs;
        bin->as.binop.right = rhs;
        lhs = bin;
    }

    return lhs;
}

AstNode *parse_expr(Parser *p) {
    return parse_precedence(p, PREC_ASSIGN);
}

// Alias para consistencia
static AstNode *parse_expression(Parser *p) {
    return parse_expr(p);
}

/* ========== Declaraciones ========== */

static AstNode *parse_var_declaration(Parser *p) {
    const Token *type_tok = expect_type(p);
    const Token *name_tok = expect_identifier(p);
    
    AstNode *var_decl = make_node_span(p, AST_VAR_DECL, type_tok, name_tok);
    var_decl->as.var_decl.type_name = intern_token_lexeme(p, type_tok); // String internada
    var_decl->as.var_decl.name = intern_token_lexeme(p, name_tok);      // String internada
    var_decl->as.var_decl.init = NULL;
    
    // Inicialización opcional
    if (tok_is(at(p), TOK_ASSIGNMENT_OP) && lexeme_is(at(p), "=")) {
        advance(p); // '='
        var_decl->as.var_decl.init = parse_expression(p);
    }
    
    expect_delim(p, ";");
    return var_decl;
}

/* Versión de declaración de variable sin esperar ';' (para FOR loops) */
static AstNode *parse_var_declaration_no_semicolon(Parser *p) {
    const Token *type_tok = expect_type(p);
    const Token *name_tok = expect_identifier(p);
    
    AstNode *var_decl = make_node_span(p, AST_VAR_DECL, type_tok, name_tok);
    var_decl->as.var_decl.type_name = intern_token_lexeme(p, type_tok); // String internada
    var_decl->as.var_decl.name = intern_token_lexeme(p, name_tok);      // String internada
    var_decl->as.var_decl.init = NULL;
    
    // Inicialización opcional
    if (tok_is(at(p), TOK_ASSIGNMENT_OP) && lexeme_is(at(p), "=")) {
        advance(p); // '='
        var_decl->as.var_decl.init = parse_expression(p);
    }
    
    return var_decl;
}

static AstNode *parse_array_declaration(Parser *p) {
    const Token *type_tok = expect_type(p);
    const Token *name_tok = expect_identifier(p);
    
    expect_delim(p, "[");
    AstNode *size_expr = parse_expression(p);
    expect_delim(p, "]");
    
    AstNode *array_decl = make_node_span(p, AST_VAR_DECL, type_tok, name_tok);
    array_decl->as.var_decl.type_name = intern_token_lexeme(p, type_tok); // String internada
    array_decl->as.var_decl.name = intern_token_lexeme(p, name_tok);      // String internada
    array_decl->as.var_decl.init = size_expr; // Almacenar tamaño en init
    
    // Lista de elementos inicial opcional
    if (is_delim(at(p), "{")) {
        AstNode *elements = parse_list_elements(p);
        // En una implementación más completa, almacenarías esto en otro campo
    }
    
    expect_delim(p, ";");
    return array_decl;
}

static AstNode *parse_list_declaration(Parser *p) {
    const Token *type_tok = expect_type(p);
    expect_kw(p, "List");
    const Token *name_tok = expect_identifier(p);
    
    AstNode *list_decl = make_node_span(p, AST_VAR_DECL, type_tok, name_tok);
    list_decl->as.var_decl.type_name = p->syms->T_List; // Usar string internada para "List"
    list_decl->as.var_decl.name = intern_token_lexeme(p, name_tok); // String internada
    list_decl->as.var_decl.init = NULL;
    
    // Lista de elementos inicial opcional
    if (is_delim(at(p), "{")) {
        list_decl->as.var_decl.init = parse_list_elements(p);
    }
    
    expect_delim(p, ";");
    return list_decl;
}

static AstNode *parse_function_declaration(Parser *p) {
    const Token *ret_type_tok = NULL;
    
    // Tipo de retorno
    if (is_type(at(p))) {
        ret_type_tok = advance(p);
    } else if (is_kw(at(p), "Null")) {
        ret_type_tok = advance(p);
    } else {
        err(p, at(p), "Se esperaba tipo de retorno de función");
        return ast_new(p->arena, AST_ERROR, span_of_tok(at(p)));
    }
    
    const Token *name_tok = expect_identifier(p);
    
    expect_delim(p, "(");
    
    AstNode *func_decl = make_node_span(p, AST_FUNC_DECL, ret_type_tok, name_tok);
    func_decl->as.func_decl.name = intern_token_lexeme(p, name_tok);         // String internada
    func_decl->as.func_decl.ret_type = intern_token_lexeme(p, ret_type_tok); // String internada
    
    // Parámetros
    if (!is_delim(at(p), ")")) {
        do {
            const Token *param_type = expect_type(p);
            const Token *param_name = expect_identifier(p);
            
            AstNode *param = make_node_span(p, AST_PARAM, param_type, param_name);
            param->as.param.type_name = intern_token_lexeme(p, param_type); // String internada
            param->as.param.name = intern_token_lexeme(p, param_name);      // String internada
            
            list_push(p, &func_decl->as.func_decl.params, param);
        } while (is_delim(at(p), ",") && advance(p));
    }
    
    expect_delim(p, ")");
    
    // Cuerpo de la función
    func_decl->as.func_decl.body = parse_block(p);
    
    expect_kw(p, "End");
    return func_decl;
}

/* ========== Sentencias y bloques ========== */

static AstNode *parse_expr_stmt(Parser *p) {
    const Token *t0 = at(p);
    AstNode *e = parse_expression(p);
    AstNode *s = make_node(p, AST_EXPR, t0);
    s->as.expr.expr = e;
    
    if (is_delim(at(p), ";")) advance(p);
    return s;
}

static AstNode *parse_return(Parser *p) {
    const Token *kw = expect_kw(p, "Return");
    AstNode *r = make_node(p, AST_RETURN, kw);
    r->as.ret.expr = NULL;
    
    if (!is_delim(at(p), ";") && !is_kw(at(p), "End") && !is_eof(p)) {
        r->as.ret.expr = parse_expression(p);
    }
    
    if (is_delim(at(p), ";")) advance(p);
    return r;
}

static AstNode *parse_print(Parser *p) {
    const Token *kw = expect_kw(p, "Print");
    expect_delim(p, "(");
    
    AstNode *pr = make_node(p, AST_PRINT, kw);
    AstList *xs = &pr->as.print.exprs;
    
    if (!is_delim(at(p), ")")) {
        do {
            AstNode *e = parse_expression(p);
            if (e) list_push(p, xs, e);
        } while (is_delim(at(p), ",") && advance(p));
    }
    
    expect_delim(p, ")");
    if (is_delim(at(p), ";")) advance(p);
    return pr;
}

static AstNode *parse_read(Parser *p) {
    const Token *kw = expect_kw(p, "Read");
    expect_delim(p, "(");
    
    AstNode *rd = make_node(p, AST_READ, kw);
    AstList *ts = &rd->as.read.targets;
    
    do {
        const Token *id = at(p);
        if (!tok_is(id, TOK_IDENTIFIER)) { 
            err(p, id, "Se esperaba identificador en Read"); 
            break; 
        }
        
        AstNode *n = make_node(p, AST_IDENT, id);
        n->as.ident.lexeme = intern_token_lexeme(p, id); // String internada
        list_push(p, ts, n);
        advance(p);
    } while (is_delim(at(p), ",") && advance(p));
    
    expect_delim(p, ")");
    if (is_delim(at(p), ";")) advance(p);
    return rd;
}

static AstNode *parse_if(Parser *p) {
    const Token *kw = expect_kw_intern(p, p->syms->K_If); // Usar versión internada
    expect_delim_intern(p, p->syms->D_LParen);           // Usar versión internada
    
    AstNode *n = make_node(p, AST_IF, kw);
    n->as.if_stmt.cond = parse_expression(p);
    
    expect_delim_intern(p, p->syms->D_RParen);           // Usar versión internada
    n->as.if_stmt.then_blk = parse_block(p);
    
    // ElseIf*
    while (is_kw_intern(p, at(p), p->syms->K_ElseIf)) {  // Usar versión internada
        const Token *ek = advance(p);
        expect_delim_intern(p, p->syms->D_LParen);       // Usar versión internada
        
        AstNode *pair = make_node(p, AST_IF, ek);
        pair->as.if_stmt.cond = parse_expression(p);
        
        expect_delim_intern(p, p->syms->D_RParen);       // Usar versión internada
        pair->as.if_stmt.then_blk = parse_block(p);
        
        list_push(p, &n->as.if_stmt.elseifs, pair);
    }
    
    // Else?
    if (is_kw(at(p),"Else")) {
        advance(p);
        n->as.if_stmt.else_blk = parse_block(p);
    }
    
    expect_kw(p, "End");
    return n;
}

static AstNode *parse_while(Parser *p) {
    const Token *kw = expect_kw(p, "While");
    expect_delim(p, "(");
    
    AstNode *n = make_node(p, AST_WHILE, kw);
    n->as.while_stmt.cond = parse_expression(p);
    
    expect_delim(p, ")");
    n->as.while_stmt.body = parse_block(p);
    
    expect_kw(p, "End");
    return n;
}

static AstNode *parse_for_stmt(Parser *p) {
    const Token *kw = expect_kw(p, "For");
    expect_delim(p, "(");
    
    AstNode *n = make_node(p, AST_FOR, kw);
    
    // Inicialización: declaración o asignación
    const Token *lookahead = peek(p, 1);
    if (is_type(at(p)) && tok_is(lookahead, TOK_IDENTIFIER)) {
        // Es una declaración de variable (sin punto y coma)
        n->as.for_stmt.init = parse_var_declaration_no_semicolon(p);
    } else {
        // Es una asignación
        n->as.for_stmt.init = parse_expression(p);
    }
    
    expect_delim(p, ":");
    
    // Condición
    n->as.for_stmt.cond = parse_expression(p);
    
    expect_delim(p, ":");
    
    // Actualización
    n->as.for_stmt.step = parse_expression(p);
    
    expect_delim(p, ")");
    
    // Cuerpo
    n->as.for_stmt.body = parse_block(p);
    
    expect_kw(p, "End");
    return n;
}

/* Bloque: consume sentencias hasta encontrar 'End' o palabras que cierren nivel superior */
AstNode *parse_block(Parser *p) {
    const Token *t0 = at(p);
    AstNode *blk = make_node(p, AST_BLOCK, t0);
    
    for (;;) {
        if (is_eof(p)) break;
        if (is_kw(at(p),"End") || is_kw(at(p),"Else") || is_kw(at(p),"ElseIf")) break;
        
        AstNode *s = parse_statement(p);
        if (s) {
            list_push(p, &blk->as.block.stmts, s);
        } else {
            advance(p); // modo pánico simple
        }
    }
    
    return blk;
}

static AstNode *parse_declaration(Parser *p) {
    const Token *t = at(p);
    const Token *next = peek(p, 1);
    const Token *next2 = peek(p, 2);
    
    if (!t) return NULL;
    
    // Detectar tipo de declaración
    if (is_type(t)) {
        if (tok_is(next, TOK_IDENTIFIER)) {
            if (is_delim(next2, "(")) {
                // Es declaración de función: tipo nombre(
                return parse_function_declaration(p);
            } else if (is_delim(next2, "[")) {
                // Es declaración de arreglo: tipo nombre[
                return parse_array_declaration(p);
            } else {
                // Es declaración de variable: tipo nombre
                return parse_var_declaration(p);
            }
        } else if (is_kw(next, "List")) {
            // Es declaración de lista: tipo List nombre
            return parse_list_declaration(p);
        }
    } else if (is_kw(t, "Null") && tok_is(next, TOK_IDENTIFIER) && is_delim(next2, "(")) {
        // Función con retorno Null: Null nombre(
        return parse_function_declaration(p);
    }
    
    return NULL; // No es una declaración
}

AstNode *parse_stmt(Parser *p) {
    return parse_statement(p);
}

static AstNode *parse_statement(Parser *p) {
    const Token *t = at(p);
    if (!t) return NULL;

    // Intentar parsear declaración primero
    AstNode *decl = parse_declaration(p);
    if (decl) return decl;

    // Sentencias de control de flujo
    if (is_kw(t,"If"))    return parse_if(p);
    if (is_kw(t,"While")) return parse_while(p);
    if (is_kw(t,"For"))   return parse_for_stmt(p);
    if (is_kw(t,"Return"))return parse_return(p);
    if (is_kw(t,"Print")) return parse_print(p);
    if (is_kw(t,"Read"))  return parse_read(p);

    // Sentencia de expresión (asignación, llamada a función, etc.)
    return parse_expr_stmt(p);
}

/* Programa: secuencia de declaraciones/sentencias hasta EOF */
AstNode *parse_program(Parser *p) {
    const Token *t0 = at(p);
    AstNode *prg = make_node(p, AST_PROGRAM, t0);
    
    while (!is_eof(p)) {
        AstNode *node = parse_statement(p);
        if (node) {
            list_push(p, &prg->as.program.decls, node);
        } else {
            advance(p); // Modo pánico
        }
    }
    
    return prg;
}
