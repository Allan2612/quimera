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

/* ============= Tests de Definición de Funciones ============= */

// Test: Declaración de función simple válida
void test_analyzer_function_declaration_valid(void) {
    const char *source = 
        "Function add(Integer a, Integer b) -> Integer "
        "Return a + b; "
        "EndFunction";
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

// Test: Función sin parámetros válida
void test_analyzer_function_no_parameters_valid(void) {
    const char *source = 
        "Function greet() -> Void "
        "Print(\"Hello, World!\"); "
        "EndFunction";
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

// Test: Función con múltiples parámetros y tipos de retorno válidos
void test_analyzer_function_multiple_params_valid(void) {
    const char *source = 
        "Function calculate(Integer x, Float y, String op) -> Float "
        "Float result = 0.0; "
        "Return result; "
        "EndFunction";
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

// Test: Función que retorna tipo incorrecto
void test_analyzer_function_wrong_return_type(void) {
    const char *source = 
        "Function getValue() -> Integer "
        "Return \"not an integer\"; "
        "EndFunction";
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

// Test: Función Void que intenta retornar valor
void test_analyzer_void_function_returns_value(void) {
    const char *source = 
        "Function doSomething() -> Void "
        "Return 42; "
        "EndFunction";
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

// Test: Función non-void sin return
void test_analyzer_non_void_function_no_return(void) {
    const char *source = 
        "Function getValue() -> Integer "
        "Print(\"No return statement\"); "
        "EndFunction";
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

/* ============= Tests de Llamadas de Funciones ============= */

// Test: Llamada de función válida
void test_analyzer_function_call_valid(void) {
    const char *source = 
        "Function add(Integer a, Integer b) -> Integer "
        "Return a + b; "
        "EndFunction "
        "Integer result = add(5, 3);";
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

// Test: Llamada con número incorrecto de argumentos
void test_analyzer_function_call_wrong_arg_count(void) {
    const char *source = 
        "Function multiply(Integer a, Integer b) -> Integer "
        "Return a * b; "
        "EndFunction "
        "Integer result = multiply(5);"; // Falta un argumento
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

// Test: Llamada con tipos de argumentos incorrectos
void test_analyzer_function_call_wrong_arg_types(void) {
    const char *source = 
        "Function concat(String a, String b) -> String "
        "Return a; " // simplificado
        "EndFunction "
        "String result = concat(\"hello\", 42);"; // Segundo argumento es Integer, no String
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

// Test: Llamada a función no declarada
void test_analyzer_function_call_undeclared(void) {
    const char *source = "Integer result = undefinedFunction(5, 3);";
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

// Test: Llamada a función con conversión automática válida
void test_analyzer_function_call_auto_conversion(void) {
    const char *source = 
        "Function process(Float x) -> Float "
        "Return x * 2.0; "
        "EndFunction "
        "Float result = process(5);"; // Integer -> Float automático
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

// Test: Función recursiva válida
void test_analyzer_recursive_function_valid(void) {
    const char *source = 
        "Function factorial(Integer n) -> Integer "
        "If (n <= 1) Return 1; End "
        "Return n * factorial(n - 1); "
        "EndFunction "
        "Integer result = factorial(5);";
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

/* ============= Tests de Sobrecarga de Funciones (si se implementa) ============= */

// Test: Funciones con mismo nombre pero diferentes parámetros
void test_analyzer_function_overload_valid(void) {
    const char *source = 
        "Function calculate(Integer x) -> Integer "
        "Return x * 2; "
        "EndFunction "
        "Function calculate(Integer x, Integer y) -> Integer "
        "Return x + y; "
        "EndFunction "
        "Integer result1 = calculate(5); "
        "Integer result2 = calculate(3, 7);";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    AstNode *program;
    SemanticAnalyzer analyzer = create_analyzer_from_source(source, &arena, &interner, &syms, &token_list, &program);
    
    bool result = analyze_program(&analyzer, program);
    
    // Nota: Dependiendo de si el lenguaje soporta sobrecarga, esto podría ser válido o inválido
    // Asumiendo que NO soporta sobrecarga por simplicidad
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(analyzer.error_count > 0);
    
    cleanup_analyzer_resources(arena, interner, syms, &token_list, &analyzer);
}

/* ============= Tests de Scope de Parámetros ============= */

// Test: Uso válido de parámetros dentro de función
void test_analyzer_parameter_scope_valid(void) {
    const char *source = 
        "Function process(Integer value, String name) -> String "
        "If (value > 0) Return name; End "
        "Return \"invalid\"; "
        "EndFunction";
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

// Test: Intento de usar parámetro fuera del scope de función
void test_analyzer_parameter_scope_invalid(void) {
    const char *source = 
        "Function process(Integer value) -> Void "
        "Print(\"Processing\"); "
        "EndFunction "
        "Integer x = value;"; // 'value' no está en scope aquí
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