#include "unity.h"
#include "analyzer.h"
#include "parser.h"
#include "lexer.h"
#include "utils.h"
#include "ast.h"
#include <string.h>
#include <stdlib.h>

// Helper para crear analizador completo desde código fuente (misma implementación)
static SemanticAnalyzer create_analyzer_from_source(const char *source, 
                                                  AstArena **arena_out, 
                                                  Interner **interner_out, 
                                                  Interned **syms_out, 
                                                  TokenList *token_list_out,
                                                  AstNode **program_out) {
    AstArena *arena = ast_arena_new();
    *arena_out = arena;

    Interner *interner = (Interner*)malloc(sizeof(Interner));
    interner_init(interner, arena, 256);
    *interner_out = interner;

    Interned *syms = (Interned*)malloc(sizeof(Interned));
    interner_bootstrap_quimera(interner, syms);
    *syms_out = syms;

    Lexer lexer;
    init_lexer(&lexer, source, interner, syms);

    init_token_list(token_list_out);
    tokenize(&lexer, token_list_out);

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

/* ============= Tests de Expresiones Aritméticas ============= */

// Test: Expresión aritmética simple válida
void test_analyzer_arithmetic_expression_valid(void) {
    const char *source = "Integer result = 2 + 3 * 4 - 1;";
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

// Test: Expresión con conversiones automáticas válidas
void test_analyzer_arithmetic_auto_conversion_valid(void) {
    const char *source = "Float result = 5 + 3.14;"; // Integer + Float -> Float
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

// Test: Expresión aritmética con tipos incompatibles
void test_analyzer_arithmetic_incompatible_types(void) {
    const char *source = "String result = \"hello\" + 42;"; // String + Integer no válido
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

// Test: División con tipos numéricos válidos
void test_analyzer_division_valid_types(void) {
    const char *source = "Float result = 10 / 3;"; // División siempre produce Float
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

// Test: División por cero literal (debería detectarse semánticamente)
void test_analyzer_division_by_zero(void) {
    const char *source = "Float result = 10 / 0;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    // Debería generar advertencia o error por división por cero
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

/* ============= Tests de Expresiones Lógicas ============= */

// Test: Expresión lógica válida
void test_analyzer_logical_expression_valid(void) {
    const char *source = 
        "Boolean a = True; Boolean b = False; "
        "Boolean result = a && b || !a;";
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

// Test: Operadores lógicos con tipos no booleanos
void test_analyzer_logical_non_boolean_operands(void) {
    const char *source = "Boolean result = 5 && 3;"; // Integer && Integer no válido
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

// Test: Operador de negación con tipo no booleano
void test_analyzer_negation_non_boolean(void) {
    const char *source = "Boolean result = !42;"; // !Integer no válido
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

/* ============= Tests de Expresiones de Comparación ============= */

// Test: Comparaciones relacionales válidas
void test_analyzer_relational_expressions_valid(void) {
    const char *source = 
        "Integer x = 10; Integer y = 5; "
        "Boolean result1 = x > y; "
        "Boolean result2 = x <= y; "
        "Boolean result3 = x != y;";
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

// Test: Comparación con conversión automática válida
void test_analyzer_comparison_auto_conversion(void) {
    const char *source = 
        "Integer x = 5; Float y = 3.14; "
        "Boolean result = x < y;"; // Integer vs Float
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

// Test: Comparación entre tipos totalmente incompatibles
void test_analyzer_comparison_incompatible_types_expr(void) {
    const char *source = 
        "String s = \"hello\"; Boolean b = True; "
        "Boolean result = s > b;"; // String vs Boolean no válido
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

/* ============= Tests de Expresiones Complejas ============= */

// Test: Expresión mixta aritmética y lógica válida
void test_analyzer_mixed_expression_valid(void) {
    const char *source = 
        "Integer a = 10; Integer b = 5; Integer c = 3; "
        "Boolean result = (a + b > c * 2) && (a % 2 == 0);";
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

// Test: Expresión anidada con paréntesis válida
void test_analyzer_nested_expression_valid(void) {
    const char *source = 
        "Float x = 2.5; Float y = 1.5; Float z = 3.0; "
        "Float result = ((x + y) * z) / (x - y);";
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

// Test: Operador unario de negación aritmética
void test_analyzer_unary_minus_valid(void) {
    const char *source = 
        "Integer x = 5; "
        "Integer negative_x = -x; "
        "Float y = -3.14;";
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

// Test: Operador unario de negación en tipo inválido
void test_analyzer_unary_minus_invalid_type(void) {
    const char *source = 
        "String s = \"hello\"; "
        "String negative_s = -s;"; // No se puede negar un String
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

/* ============= Tests de Precedencia de Operadores ============= */

// Test: Precedencia de operadores aritméticos correcta
void test_analyzer_operator_precedence_arithmetic(void) {
    const char *source = 
        "Integer result = 2 + 3 * 4;"; // Debe ser 2 + (3 * 4) = 14, no (2 + 3) * 4 = 20
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

// Test: Precedencia de operadores lógicos y relacionales
void test_analyzer_operator_precedence_logical(void) {
    const char *source = 
        "Integer x = 5; Integer y = 3; Integer z = 7; "
        "Boolean result = x > y && y < z;"; // Debe ser (x > y) && (y < z)
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