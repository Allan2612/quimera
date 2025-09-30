// tests/test_runner.c
#include "unity.h"

// prototipos (de lexer_test.c)
void test_lexer_init(void);
void test_token_list_operations(void);
void test_tokenize_simple_identifier(void);
void test_tokenize_identifier_with_underscore(void);
void test_tokenize_identifier_with_numbers(void);
void test_tokenize_keyword_if(void);
void test_tokenize_keyword_else(void);
void test_tokenize_integer(void);
void test_tokenize_float_number(void);
void test_tokenize_invalid_double_dot_number(void);
void test_tokenize_zero(void);
void test_tokenize_multiple_tokens(void);
void test_tokenize_identifier_and_number_mixed(void);
void test_tokenize_with_spaces(void);
void test_tokenize_with_tabs_and_newlines(void);
void test_tokenize_empty_string(void);
void test_tokenize_only_spaces(void);
void test_lexer_position_tracking(void);

void register_all_tests(void) {
    // Básicos
    RUN_TEST(test_lexer_init);
    RUN_TEST(test_token_list_operations);

    // Identificadores
    RUN_TEST(test_tokenize_simple_identifier);
    RUN_TEST(test_tokenize_identifier_with_underscore);
    RUN_TEST(test_tokenize_identifier_with_numbers);

    // Palabras reservadas
    RUN_TEST(test_tokenize_keyword_if);
    RUN_TEST(test_tokenize_keyword_else);

    // Números
    RUN_TEST(test_tokenize_integer);
    RUN_TEST(test_tokenize_float_number);
    RUN_TEST(test_tokenize_invalid_double_dot_number);
    RUN_TEST(test_tokenize_zero);

    // Varios tokens / espacios / edge
    RUN_TEST(test_tokenize_multiple_tokens);
    RUN_TEST(test_tokenize_identifier_and_number_mixed);
    RUN_TEST(test_tokenize_with_spaces);
    RUN_TEST(test_tokenize_with_tabs_and_newlines);
    RUN_TEST(test_tokenize_empty_string);
    RUN_TEST(test_tokenize_only_spaces);

    // Posicionamiento
    RUN_TEST(test_lexer_position_tracking);
}
