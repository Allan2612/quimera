#include "unity.h"
#include "parser.h"
#include "lexer.h"
#include "utils.h"
#include "ast.h"
#include <string.h>
#include <stdlib.h>

// Helper para crear parser desde código fuente
static Parser create_parser_from_source(const char *source, AstArena **arena_out) {
    Lexer lexer;
    TokenList token_list;
    
    init_lexer(&lexer, source);
    init_token_list(&token_list);
    tokenize(&lexer, &token_list);
    
    AstArena *arena = ast_arena_new();
    *arena_out = arena;
    
    Parser parser = {
        .toks = token_list.tokens,
        .count = token_list.count,
        .i = 0,
        .arena = arena,
        .error_count = 0
    };
    
    return parser;
}

// Helper para limpiar recursos
static void cleanup_parser(AstArena *arena, TokenList *tokens) {
    if (arena) ast_arena_free(arena);
    if (tokens) free_token_list(tokens);
}

/* ============= Tests de Declaraciones ============= */

void test_parser_simple_variable_declaration(void) {
    const char *source = "Integer x;";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(AST_PROGRAM, program->kind);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *decl = program->as.program.decls.data[0];
    TEST_ASSERT_NOT_NULL(decl);
    TEST_ASSERT_EQUAL(AST_VAR_DECL, decl->kind);
    TEST_ASSERT_EQUAL_STRING("Integer", decl->as.var_decl.type_name);
    TEST_ASSERT_EQUAL_STRING("x", decl->as.var_decl.name);
    TEST_ASSERT_NULL(decl->as.var_decl.init);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_variable_with_initialization(void) {
    const char *source = "Integer x = 42;";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *decl = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_VAR_DECL, decl->kind);
    TEST_ASSERT_NOT_NULL(decl->as.var_decl.init);
    TEST_ASSERT_EQUAL(AST_LITERAL, decl->as.var_decl.init->kind);
    TEST_ASSERT_EQUAL(LIT_INT, decl->as.var_decl.init->as.literal.tag);
    TEST_ASSERT_EQUAL(42, decl->as.var_decl.init->as.literal.val.i);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_array_declaration(void) {
    const char *source = "Integer arr[10];";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *decl = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_VAR_DECL, decl->kind);
    TEST_ASSERT_EQUAL_STRING("Integer", decl->as.var_decl.type_name);
    TEST_ASSERT_EQUAL_STRING("arr", decl->as.var_decl.name);
    TEST_ASSERT_NOT_NULL(decl->as.var_decl.init); // Tamaño del array
    TEST_ASSERT_EQUAL(AST_LITERAL, decl->as.var_decl.init->kind);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_list_declaration(void) {
    const char *source = "Integer List numbers;";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *decl = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_VAR_DECL, decl->kind);
    TEST_ASSERT_EQUAL_STRING("List", decl->as.var_decl.type_name);
    TEST_ASSERT_EQUAL_STRING("numbers", decl->as.var_decl.name);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_function_declaration(void) {
    const char *source = "Integer add() Return 0; End";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *func = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_FUNC_DECL, func->kind);
    TEST_ASSERT_EQUAL_STRING("Integer", func->as.func_decl.ret_type);
    TEST_ASSERT_EQUAL_STRING("add", func->as.func_decl.name);
    TEST_ASSERT_EQUAL(0, func->as.func_decl.params.count);
    TEST_ASSERT_NOT_NULL(func->as.func_decl.body);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_function_with_parameters(void) {
    const char *source = "Integer add(Integer a, Integer b) Return a + b; End";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *func = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_FUNC_DECL, func->kind);
    TEST_ASSERT_EQUAL(2, func->as.func_decl.params.count);
    
    AstNode *param1 = func->as.func_decl.params.data[0];
    TEST_ASSERT_EQUAL(AST_PARAM, param1->kind);
    TEST_ASSERT_EQUAL_STRING("Integer", param1->as.param.type_name);
    TEST_ASSERT_EQUAL_STRING("a", param1->as.param.name);
    
    AstNode *param2 = func->as.func_decl.params.data[1];
    TEST_ASSERT_EQUAL(AST_PARAM, param2->kind);
    TEST_ASSERT_EQUAL_STRING("Integer", param2->as.param.type_name);
    TEST_ASSERT_EQUAL_STRING("b", param2->as.param.name);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_function_with_return_type_null(void) {
    const char *source = "Null main() Print(42); End";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *func = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_FUNC_DECL, func->kind);
    TEST_ASSERT_EQUAL_STRING("Null", func->as.func_decl.ret_type);
    TEST_ASSERT_EQUAL_STRING("main", func->as.func_decl.name);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

/* ============= Tests de Expresiones ============= */

void test_parser_arithmetic_expressions(void) {
    const char *source = "x = 2 + 3 * 4;";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_EXPR, stmt->kind);
    
    AstNode *assign = stmt->as.expr.expr;
    TEST_ASSERT_EQUAL(AST_ASSIGN, assign->kind);
    TEST_ASSERT_EQUAL(OP_ASSIGN, assign->as.assign.op);
    
    // El lado derecho debe ser una expresión binaria: 2 + (3 * 4)
    AstNode *rhs = assign->as.assign.rhs;
    TEST_ASSERT_EQUAL(AST_BINOP, rhs->kind);
    TEST_ASSERT_EQUAL(OP_ADD, rhs->as.binop.op);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_logical_expressions(void) {
    const char *source = "result = a > 5 And b < 10;";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *stmt = program->as.program.decls.data[0];
    AstNode *assign = stmt->as.expr.expr;
    AstNode *rhs = assign->as.assign.rhs;
    
    TEST_ASSERT_EQUAL(AST_BINOP, rhs->kind);
    TEST_ASSERT_EQUAL(OP_AND, rhs->as.binop.op);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_comparison_expressions(void) {
    const char *source = "flag = x == 42;";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *stmt = program->as.program.decls.data[0];
    AstNode *assign = stmt->as.expr.expr;
    AstNode *rhs = assign->as.assign.rhs;
    
    TEST_ASSERT_EQUAL(AST_BINOP, rhs->kind);
    TEST_ASSERT_EQUAL(OP_EQ, rhs->as.binop.op);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_function_calls(void) {
    const char *source = "result = add(5, 10);";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *stmt = program->as.program.decls.data[0];
    AstNode *assign = stmt->as.expr.expr;
    AstNode *rhs = assign->as.assign.rhs;
    
    TEST_ASSERT_EQUAL(AST_CALL, rhs->kind);
    TEST_ASSERT_EQUAL(AST_IDENT, rhs->as.call.callee->kind);
    TEST_ASSERT_EQUAL(2, rhs->as.call.args.count);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_array_indexing(void) {
    const char *source = "value = arr[5];";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *stmt = program->as.program.decls.data[0];
    AstNode *assign = stmt->as.expr.expr;
    AstNode *rhs = assign->as.assign.rhs;
    
    TEST_ASSERT_EQUAL(AST_INDEX, rhs->kind);
    TEST_ASSERT_EQUAL(AST_IDENT, rhs->as.index.base->kind);
    TEST_ASSERT_EQUAL(AST_LITERAL, rhs->as.index.index->kind);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_list_methods(void) {
    const char *source = "size = list.size();";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *stmt = program->as.program.decls.data[0];
    AstNode *assign = stmt->as.expr.expr;
    AstNode *rhs = assign->as.assign.rhs;
    
    TEST_ASSERT_EQUAL(AST_CALL, rhs->kind);
    TEST_ASSERT_EQUAL(AST_IDENT, rhs->as.call.callee->kind);
    TEST_ASSERT_EQUAL(1, rhs->as.call.args.count); // El objeto lista como argumento
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_precedence(void) {
    const char *source = "result = 2 + 3 * 4 + 5;"; // Debe ser ((2 + (3 * 4)) + 5)
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *stmt = program->as.program.decls.data[0];
    AstNode *assign = stmt->as.expr.expr;
    AstNode *rhs = assign->as.assign.rhs;
    
    // Debe ser suma en el nivel superior
    TEST_ASSERT_EQUAL(AST_BINOP, rhs->kind);
    TEST_ASSERT_EQUAL(OP_ADD, rhs->as.binop.op);
    
    // El lado izquierdo debe ser otra suma
    TEST_ASSERT_EQUAL(AST_BINOP, rhs->as.binop.left->kind);
    TEST_ASSERT_EQUAL(OP_ADD, rhs->as.binop.left->as.binop.op);
    
    // El lado derecho del primer binop debe ser multiplicación
    TEST_ASSERT_EQUAL(AST_BINOP, rhs->as.binop.left->as.binop.right->kind);
    TEST_ASSERT_EQUAL(OP_MUL, rhs->as.binop.left->as.binop.right->as.binop.op);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_parentheses(void) {
    const char *source = "result = (2 + 3) * 4;";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *stmt = program->as.program.decls.data[0];
    AstNode *assign = stmt->as.expr.expr;
    AstNode *rhs = assign->as.assign.rhs;
    
    // Debe ser multiplicación en el nivel superior
    TEST_ASSERT_EQUAL(AST_BINOP, rhs->kind);
    TEST_ASSERT_EQUAL(OP_MUL, rhs->as.binop.op);
    
    // El lado izquierdo debe ser suma (por los paréntesis)
    TEST_ASSERT_EQUAL(AST_BINOP, rhs->as.binop.left->kind);
    TEST_ASSERT_EQUAL(OP_ADD, rhs->as.binop.left->as.binop.op);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_unary_operators(void) {
    const char *source = "result = -x + Not flag;";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *stmt = program->as.program.decls.data[0];
    AstNode *assign = stmt->as.expr.expr;
    AstNode *rhs = assign->as.assign.rhs;
    
    TEST_ASSERT_EQUAL(AST_BINOP, rhs->kind);
    TEST_ASSERT_EQUAL(OP_ADD, rhs->as.binop.op);
    
    // Lado izquierdo debe ser unario negativo
    TEST_ASSERT_EQUAL(AST_UNOP, rhs->as.binop.left->kind);
    TEST_ASSERT_EQUAL(OP_NEG, rhs->as.binop.left->as.unop.op);
    
    // Lado derecho debe ser unario NOT
    TEST_ASSERT_EQUAL(AST_UNOP, rhs->as.binop.right->kind);
    TEST_ASSERT_EQUAL(OP_NOT, rhs->as.binop.right->as.unop.op);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}

void test_parser_assignment_operators(void) {
    const char *source = "x += 5;";
    AstArena *arena;
    Parser p = create_parser_from_source(source, &arena);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *stmt = program->as.program.decls.data[0];
    AstNode *assign = stmt->as.expr.expr;
    
    TEST_ASSERT_EQUAL(AST_ASSIGN, assign->kind);
    TEST_ASSERT_EQUAL(OP_ADD_ASSIGN, assign->as.assign.op);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    ast_arena_free(arena);
}