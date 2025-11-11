#include "unity.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include <string.h>

static AstArena *arena;
static Interner interner;
static Interned symbols;

void setUp(void) {
    arena = ast_arena_new();
    TEST_ASSERT_NOT_NULL(arena);
    
    interner_init(&interner, arena, 64);
    interner_bootstrap_quimera(&interner, &symbols);
}

void tearDown(void) {
    interner_destroy(&interner);
    ast_arena_free(arena);
}

static AstNode* parse_source(const char *source) {
    Lexer lexer;
    init_lexer(&lexer, source, &interner, &symbols);
    
    TokenList tokens;
    init_token_list(&tokens);
    tokenize(&lexer, &tokens);
    
    Parser parser = {
        .toks = tokens.tokens,
        .count = tokens.count,
        .i = 0,
        .arena = arena,
        .interner = &interner,
        .syms = &symbols,
        .error_count = 0
    };
    
    AstNode *program = parse_program(&parser);
    free_token_list(&tokens);
    
    return program;
}

void test_arithmetic_operations(void) {
    const char *source = "Integer result = 10 + 5 * 2;";
    AstNode *program = parse_source(source);
    TEST_ASSERT_NOT_NULL(program);
    
    InterpreterContext ctx;
    interpreter_init(&ctx, arena, &interner, &symbols);
    
    EvalResult result = interpret_program(&ctx, program);
    TEST_ASSERT_EQUAL(FLOW_NORMAL, result.flow);
    TEST_ASSERT_FALSE(ctx.has_error);
    
    // Verificar que la variable 'result' tiene valor 20 (10 + 5*2)
    const char *result_name = interner_intern_cstr(&interner, "result");
    Value *value = scope_lookup(ctx.current_scope, result_name);
    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_EQUAL(VAL_INTEGER, value->type);
    TEST_ASSERT_EQUAL(20, value->as.integer);
    
    interpreter_destroy(&ctx);
}

void test_boolean_operations(void) {
    const char *source = 
        "Boolean a = True;\n"
        "Boolean b = False;\n"
        "Boolean and_result = a And b;\n"
        "Boolean or_result = a Or b;\n"
        "Boolean not_result = Not a;\n";
        
    AstNode *program = parse_source(source);
    TEST_ASSERT_NOT_NULL(program);
    
    InterpreterContext ctx;
    interpreter_init(&ctx, arena, &interner, &symbols);
    
    EvalResult result = interpret_program(&ctx, program);
    TEST_ASSERT_EQUAL(FLOW_NORMAL, result.flow);
    TEST_ASSERT_FALSE(ctx.has_error);
    
    // Verificar operación AND
    Value *and_val = scope_lookup(ctx.current_scope, interner_intern_cstr(&interner, "and_result"));
    TEST_ASSERT_NOT_NULL(and_val);
    TEST_ASSERT_EQUAL(VAL_BOOLEAN, and_val->type);
    TEST_ASSERT_FALSE(and_val->as.boolean);
    
    // Verificar operación OR
    Value *or_val = scope_lookup(ctx.current_scope, interner_intern_cstr(&interner, "or_result"));
    TEST_ASSERT_NOT_NULL(or_val);
    TEST_ASSERT_EQUAL(VAL_BOOLEAN, or_val->type);
    TEST_ASSERT_TRUE(or_val->as.boolean);
    
    // Verificar operación NOT
    Value *not_val = scope_lookup(ctx.current_scope, interner_intern_cstr(&interner, "not_result"));
    TEST_ASSERT_NOT_NULL(not_val);
    TEST_ASSERT_EQUAL(VAL_BOOLEAN, not_val->type);
    TEST_ASSERT_FALSE(not_val->as.boolean);
    
    interpreter_destroy(&ctx);
}

void test_type_promotion(void) {
    const char *source = "Float result = 10 + 3.14;"; // Integer + Float → Float
    AstNode *program = parse_source(source);
    TEST_ASSERT_NOT_NULL(program);
    
    InterpreterContext ctx;
    interpreter_init(&ctx, arena, &interner, &symbols);
    
    EvalResult result = interpret_program(&ctx, program);
    TEST_ASSERT_EQUAL(FLOW_NORMAL, result.flow);
    TEST_ASSERT_FALSE(ctx.has_error);
    
    Value *value = scope_lookup(ctx.current_scope, interner_intern_cstr(&interner, "result"));
    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_EQUAL(VAL_FLOAT, value->type);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 13.14, value->as.float_val);
    
    interpreter_destroy(&ctx);
}

void test_string_operations(void) {
    const char *source = "String greeting = \"Hello\" + \" \" + \"World\";";
    AstNode *program = parse_source(source);
    TEST_ASSERT_NOT_NULL(program);
    
    InterpreterContext ctx;
    interpreter_init(&ctx, arena, &interner, &symbols);
    
    EvalResult result = interpret_program(&ctx, program);
    TEST_ASSERT_EQUAL(FLOW_NORMAL, result.flow);
    TEST_ASSERT_FALSE(ctx.has_error);
    
    Value *value = scope_lookup(ctx.current_scope, interner_intern_cstr(&interner, "greeting"));
    TEST_ASSERT_NOT_NULL(value);
    TEST_ASSERT_EQUAL(VAL_STRING, value->type);
    TEST_ASSERT_EQUAL_STRING("Hello World", value->as.string);
    
    interpreter_destroy(&ctx);
}

void test_comparisons(void) {
    const char *source = 
        "Integer a = 10;\n"
        "Integer b = 5;\n"
        "Boolean greater = a > b;\n"
        "Boolean equal = a == b;\n"
        "Boolean less_equal = a <= b;\n";
        
    AstNode *program = parse_source(source);
    TEST_ASSERT_NOT_NULL(program);
    
    InterpreterContext ctx;
    interpreter_init(&ctx, arena, &interner, &symbols);
    
    EvalResult result = interpret_program(&ctx, program);
    TEST_ASSERT_EQUAL(FLOW_NORMAL, result.flow);
    TEST_ASSERT_FALSE(ctx.has_error);
    
    Value *greater = scope_lookup(ctx.current_scope, interner_intern_cstr(&interner, "greater"));
    TEST_ASSERT_NOT_NULL(greater);
    TEST_ASSERT_TRUE(greater->as.boolean);
    
    Value *equal = scope_lookup(ctx.current_scope, interner_intern_cstr(&interner, "equal"));
    TEST_ASSERT_NOT_NULL(equal);
    TEST_ASSERT_FALSE(equal->as.boolean);
    
    Value *less_equal = scope_lookup(ctx.current_scope, interner_intern_cstr(&interner, "less_equal"));
    TEST_ASSERT_NOT_NULL(less_equal);
    TEST_ASSERT_FALSE(less_equal->as.boolean);
    
    interpreter_destroy(&ctx);
}

void test_assignment_operators(void) {
    const char *source = 
        "Integer x = 10;\n"
        "x += 5;\n";  // x debería ser 15
        
    AstNode *program = parse_source(source);
    TEST_ASSERT_NOT_NULL(program);
    
    InterpreterContext ctx;
    interpreter_init(&ctx, arena, &interner, &symbols);
    
    EvalResult result = interpret_program(&ctx, program);
    TEST_ASSERT_EQUAL(FLOW_NORMAL, result.flow);
    TEST_ASSERT_FALSE(ctx.has_error);
    
    Value *x = scope_lookup(ctx.current_scope, interner_intern_cstr(&interner, "x"));
    TEST_ASSERT_NOT_NULL(x);
    TEST_ASSERT_EQUAL(VAL_INTEGER, x->type);
    TEST_ASSERT_EQUAL(15, x->as.integer);
    
    interpreter_destroy(&ctx);
}

void test_null_values(void) {
    const char *source = 
        "Integer x;\n"  // Sin inicializar → Null
        "Boolean is_null = x == Null;\n";
        
    AstNode *program = parse_source(source);
    TEST_ASSERT_NOT_NULL(program);
    
    InterpreterContext ctx;
    interpreter_init(&ctx, arena, &interner, &symbols);
    
    EvalResult result = interpret_program(&ctx, program);
    TEST_ASSERT_EQUAL(FLOW_NORMAL, result.flow);
    TEST_ASSERT_FALSE(ctx.has_error);
    
    Value *x = scope_lookup(ctx.current_scope, interner_intern_cstr(&interner, "x"));
    TEST_ASSERT_NOT_NULL(x);
    TEST_ASSERT_EQUAL(VAL_NULL, x->type);
    
    Value *is_null = scope_lookup(ctx.current_scope, interner_intern_cstr(&interner, "is_null"));
    TEST_ASSERT_NOT_NULL(is_null);
    TEST_ASSERT_EQUAL(VAL_BOOLEAN, is_null->type);
    TEST_ASSERT_TRUE(is_null->as.boolean);
    
    interpreter_destroy(&ctx);
}

void test_truthiness(void) {
    // Test de truthiness para diferentes tipos
    Value null_val = value_null();
    TEST_ASSERT_FALSE(value_is_truthy(&null_val));
    
    Value zero_int = value_integer(0);
    TEST_ASSERT_FALSE(value_is_truthy(&zero_int));
    
    Value zero_float = value_float(0.0);
    TEST_ASSERT_FALSE(value_is_truthy(&zero_float));
    
    Value empty_string = value_string("");
    TEST_ASSERT_FALSE(value_is_truthy(&empty_string));
    
    Value false_bool = value_boolean(false);
    TEST_ASSERT_FALSE(value_is_truthy(&false_bool));
    
    // Valores que son truthy
    Value nonzero_int = value_integer(42);
    TEST_ASSERT_TRUE(value_is_truthy(&nonzero_int));
    
    Value nonzero_float = value_float(3.14);
    TEST_ASSERT_TRUE(value_is_truthy(&nonzero_float));
    
    Value nonempty_string = value_string("hello");
    TEST_ASSERT_TRUE(value_is_truthy(&nonempty_string));
    
    Value true_bool = value_boolean(true);
    TEST_ASSERT_TRUE(value_is_truthy(&true_bool));
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_arithmetic_operations);
    RUN_TEST(test_boolean_operations);
    RUN_TEST(test_type_promotion);
    RUN_TEST(test_string_operations);
    RUN_TEST(test_comparisons);
    RUN_TEST(test_assignment_operators);
    RUN_TEST(test_null_values);
    RUN_TEST(test_truthiness);
    
    return UNITY_END();
}