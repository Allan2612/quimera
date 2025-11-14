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

/* ============= Tests de Comparaciones ============= */

// Test: Comparación válida entre tipos compatibles
void test_analyzer_comparison_compatible_types(void) {
    const char *source = "Integer x = 5; Integer y = 3; Boolean result = (x > y);";
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

// Test: Comparación de igualdad Char == String (permitida)
void test_analyzer_char_string_equality_valid(void) {
    const char *source = "Char c = 'a'; String s = \"a\"; Boolean result = (c == s);";
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

// Test: Comparación entre tipos incompatibles
void test_analyzer_comparison_incompatible_types(void) {
    const char *source = "Integer x = 10; String s = \"10\"; Boolean result = (x == s);";
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

// Test: Comparación Boolean == Integer (no permitida)
void test_analyzer_boolean_integer_comparison_error(void) {
    const char *source = "Boolean b = True; Integer x = 1; Boolean result = (b == x);";
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

/* ============= Tests de Estructuras de Control ============= */

// Test: IF con condición booleana válida
void test_analyzer_if_boolean_condition_valid(void) {
    const char *source = 
        "Boolean flag = True; "
        "If (flag) Print(\"True\"); End";
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

// Test: IF con condición no booleana
void test_analyzer_if_non_boolean_condition_error(void) {
    const char *source = 
        "Integer x = 5; "
        "If (x) Print(\"Non-zero\"); End";
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

// Test: WHILE con condición booleana válida
void test_analyzer_while_boolean_condition_valid(void) {
    const char *source = 
        "Integer i = 0; "
        "While (i < 10) i = i + 1; End";
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

// Test: WHILE con condición no booleana
void test_analyzer_while_non_boolean_condition_error(void) {
    const char *source = 
        "Integer i = 5; "
        "While (i) i = i - 1; End";
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

// Test: FOR con condición booleana válida
void test_analyzer_for_boolean_condition_valid(void) {
    const char *source = 
        "For (Integer i = 0 : i < 10 : i = i + 1) "
        "Print(i); "
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

// Test: FOR con condición no booleana
void test_analyzer_for_non_boolean_condition_error(void) {
    const char *source = 
        "For (Integer i = 10 : i : i = i - 1) "
        "Print(i); "
        "End";
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

/* ============= Tests de Asignaciones ============= */

// Test: Asignación válida
void test_analyzer_valid_assignment(void) {
    const char *source = "Integer x = 5; x = 10;";
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

// Test: Asignación a tipo incompatible
void test_analyzer_invalid_assignment(void) {
    const char *source = "String s = \"hello\"; s = 42;";
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

// Test: Asignación compuesta válida
void test_analyzer_compound_assignment_valid(void) {
    const char *source = "Integer x = 5; x += 3;";
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

// Test: Asignación compuesta con tipos incompatibles
void test_analyzer_compound_assignment_invalid(void) {
    const char *source = "String s = \"hello\"; s += 42;";
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

// Test: Asignación a expresión no válida
void test_analyzer_assignment_to_non_lvalue(void) {
    const char *source = "Integer x = 5; Integer y = 3; (x + y) = 10;";
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