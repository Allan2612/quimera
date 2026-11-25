#include "unity.h"
#include "analyzer.h"
#include "parser.h"
#include "lexer.h"
#include "utils.h"
#include "ast.h"
#include <string.h>
#include <stdlib.h>

// Helper para crear analizador completo desde código fuente
static SemanticAnalyzer create_analyzer_from_source(const char *source, 
                                                  AstArena **arena_out, 
                                                  Interner **interner_out, 
                                                  Interned **syms_out, 
                                                  TokenList *token_list_out,
                                                  AstNode **program_out) {
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
    
    *program_out = parse_program(&parser);

    // 5) Analyzer
    SemanticAnalyzer analyzer;
    analyzer_init(&analyzer, arena, interner, syms);
    
    return analyzer;
}

// Helper para limpiar recursos
static void cleanup_analyzer_resources(AstArena *arena, Interner *interner, Interned *syms, TokenList *tokens, SemanticAnalyzer *analyzer) {
    if (analyzer) analyzer_destroy(analyzer);
    if (arena) ast_arena_free(arena);
    if (interner) {
        interner_destroy(interner);
        free(interner);
    }
    if (syms) free(syms);
    if (tokens) free_token_list(tokens);
}

/* ============= Tests de Tipos y Conversiones ============= */

// Test: Conversión implícita Integer -> Float
void test_analyzer_integer_to_float_conversion(void) {
    const char *source = "Integer x = 5; Float y = x;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Conversión implícita Char -> String
void test_analyzer_char_to_string_conversion(void) {
    const char *source = "Char c = 'A'; String s = c;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Conversión no permitida Float -> Integer
void test_analyzer_float_to_integer_error(void) {
    const char *source = "Float f = 3.14; Integer i = f;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Boolean usado en operación aritmética
void test_analyzer_boolean_arithmetic_error(void) {
    const char *source = "Boolean b = True; Integer x = b + 1;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

/* ============= Tests de Declaraciones y Alcance ============= */

// Test: Variable no declarada
void test_analyzer_undeclared_variable(void) {
    const char *source = "Integer x = y + 1;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Redeclaración en mismo ámbito
void test_analyzer_redeclaration_error(void) {
    const char *source = "Integer x = 5; Integer x = 10;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Variable válida en ámbito diferente
void test_analyzer_different_scope_declaration(void) {
    const char *source = 
        "Integer x = 5; "
        "If (x > 0) "
        "Integer x = 10; "
        "Print(x); "
        "End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Uso de palabra reservada como identificador
void test_analyzer_reserved_word_error(void) {
    const char *source = "Integer If = 5;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

/* ============= Tests de Operaciones Aritméticas ============= */

// Test: Operación aritmética válida entre enteros
void test_analyzer_integer_arithmetic_valid(void) {
    const char *source = "Integer x = 5; Integer y = 3; Integer z = x + y * 2;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Operación aritmética entre Integer y Float
void test_analyzer_mixed_arithmetic_valid(void) {
    const char *source = "Integer x = 5; Float y = 3.14; Float z = x + y;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Operación aritmética con tipos incompatibles
void test_analyzer_arithmetic_type_error(void) {
    const char *source = "String s = \"hello\"; Integer x = s + 5;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

/* ============= Tests de Operaciones Lógicas ============= */

// Test: Operación lógica válida
void test_analyzer_logical_operation_valid(void) {
    const char *source = "Boolean a = True; Boolean b = False; Boolean c = a And b;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Operación lógica con tipos no booleanos
void test_analyzer_logical_operation_error(void) {
    const char *source = "Integer x = 5; Integer y = 3; Boolean z = x And y;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Negación lógica válida
void test_analyzer_logical_negation_valid(void) {
    const char *source = "Boolean flag = True; Boolean result = Not flag;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Negación lógica con tipo incorrecto
void test_analyzer_logical_negation_error(void) {
    const char *source = "Integer x = 5; Boolean result = Not x;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

/* ============= Tests de Declaraciones Básicas ============= */

// Test: Declaración válida de Integer
void test_analyzer_valid_integer_declaration(void) {
    const char *source = "Integer x = 42;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Declaración válida de Float
void test_analyzer_valid_float_declaration(void) {
    const char *source = "Float x = 3.14;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Declaración válida de String
void test_analyzer_valid_string_declaration(void) {
    const char *source = "String s = \"Hello\";";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Declaración válida de Char
void test_analyzer_valid_char_declaration(void) {
    const char *source = "Char c = 'A';";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Declaración válida de Boolean
void test_analyzer_valid_boolean_declaration(void) {
    const char *source = "Boolean flag = True;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Error de uso antes de declaración
void test_analyzer_use_before_declaration(void) {
    const char *source = "Integer x = y; Integer y = 5;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Error de asignación de tipos incompatibles
void test_analyzer_type_mismatch_assignment(void) {
    const char *source = "Integer x = \"hello\";";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Conversión inválida Float -> Integer
void test_analyzer_invalid_float_to_integer_conversion(void) {
    const char *source = "Float f = 3.14; Integer x = f;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Error de aritmética con Boolean
void test_analyzer_invalid_boolean_arithmetic(void) {
    const char *source = "Boolean a = True; Boolean b = False; Boolean result = a + b;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Operaciones aritméticas válidas
void test_analyzer_valid_arithmetic_operations(void) {
    const char *source = "Integer a = 10; Integer b = 5; Integer sum = a + b; Integer diff = a - b; Integer prod = a * b; Integer quot = a / b;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Operaciones lógicas válidas
void test_analyzer_valid_logical_operations(void) {
    const char *source = "Boolean a = True; Boolean b = False; Boolean and_result = a And b; Boolean or_result = a Or b; Boolean not_result = Not a;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Shadowing de variables en ámbitos anidados
void test_analyzer_scope_variable_shadowing(void) {
    const char *source = "Integer x = 10; If (True) Integer x = 20; Print(x); End Print(x);";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

// Test: Bloques anidados con ámbitos válidos
void test_analyzer_scope_nested_blocks(void) {
    const char *source = "If (True) Integer a = 1; If (True) Integer b = 2; Integer c = a + b; End End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, analyzer.error_count);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}