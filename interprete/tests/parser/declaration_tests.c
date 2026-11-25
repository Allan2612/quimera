#include "unity.h"
#include "parser.h"
#include "lexer.h"
#include "utils.h"
#include "ast.h"
#include <string.h>
#include <stdlib.h>

// Helper para crear parser desde código fuente (misma implementación que error_handling_tests.c)
static Parser create_parser_from_source(const char *source, AstArena **arena_out, Interner **interner_out, Interned **syms_out, TokenList *token_list_out) {
    // 1) Arena
    AstArena *arena = ast_arena_new();
    *arena_out = arena;

    // 2) Interner + símbolos internados
    Interner *interner = (Interner*)malloc(sizeof(Interner));
    interner_init(interner, arena, 256);
    *interner_out = interner;

    Interned *syms = (Interned*)malloc(sizeof(Interned));
    interner_bootstrap_quimera(interner, syms);
    *syms_out = syms;

    // 3) Lexer + tokens
    Lexer lexer;
    init_lexer(&lexer, source, interner, syms);

    init_token_list(token_list_out);
    tokenize(&lexer, token_list_out);

    // 4) Parser
    Parser parser = {
        .toks        = token_list_out->tokens,
        .count       = token_list_out->count,
        .i           = 0,
        .arena       = arena,
        .interner    = interner,
        .syms        = syms,
        .error_count = 0
    };
    return parser;
}

// Helper para limpiar recursos
static void cleanup_parser_resources(AstArena *arena, Interner *interner, Interned *syms, TokenList *tokens) {
    if (arena) ast_arena_free(arena);
    if (interner) {
        interner_destroy(interner);
        free(interner);
    }
    if (syms) free(syms);
    if (tokens) free_token_list(tokens);
}

/* ============= Tests de Declaraciones de Variables ============= */

// Test: Declaración simple de variable entera
void test_parser_var_declaration_integer(void) {
    const char *source = "Integer x;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *var_decl = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_VAR_DECL, var_decl->kind);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: Declaración de variable con inicialización
void test_parser_var_declaration_with_init(void) {
    const char *source = "Integer x = 42;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *var_decl = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_VAR_DECL, var_decl->kind);
    TEST_ASSERT_NOT_NULL(var_decl->as.var_decl.init);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: Declaración de variable Float
void test_parser_var_declaration_float(void) {
    const char *source = "Float pi = 3.14;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *var_decl = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_VAR_DECL, var_decl->kind);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: Declaración de variable String
void test_parser_var_declaration_string(void) {
    const char *source = "String name = \"Quimera\";";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: Declaración de variable Boolean
void test_parser_var_declaration_boolean(void) {
    const char *source = "Boolean flag = True;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: Declaración de variable Char
void test_parser_var_declaration_char(void) {
    const char *source = "Char letter = 'A';";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

/* ============= Tests de Declaraciones de Arrays ============= */

// Test: Declaración de array de enteros
void test_parser_array_declaration_integer(void) {
    const char *source = "Integer arr[10];";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *arr_decl = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_VAR_DECL, arr_decl->kind);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: Declaración de array de strings
void test_parser_array_declaration_string(void) {
    const char *source = "String names[5];";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

/* ============= Tests de Declaraciones de Listas ============= */

// Test: Declaración de lista de enteros
void test_parser_list_declaration_integer(void) {
    const char *source = "Integer List numbers;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: Declaración de lista de strings
void test_parser_list_declaration_string(void) {
    const char *source = "String List words;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

/* ============= Tests de Declaraciones de Funciones ============= */

// Test: Función simple sin parámetros
void test_parser_function_declaration_simple(void) {
    const char *source = "Null sayHello() Print(\"Hello\"); End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *func_decl = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_FUNC_DECL, func_decl->kind);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: Función con retorno y parámetros
void test_parser_function_declaration_with_params(void) {
    const char *source = "Integer add(Integer a, Integer b) Return a + b; End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *func_decl = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_FUNC_DECL, func_decl->kind);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

/* ============= Tests de Múltiples Declaraciones ============= */

// Test: Múltiples declaraciones válidas
void test_parser_multiple_declarations(void) {
    const char *source = "Integer x = 5; Float y = 3.14; String name = \"test\";";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(3, program->as.program.decls.count);
    
    // Verificar que todas son declaraciones de variables
    for (int i = 0; i < 3; i++) {
        AstNode *decl = program->as.program.decls.data[i];
        TEST_ASSERT_EQUAL(AST_VAR_DECL, decl->kind);
    }
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}