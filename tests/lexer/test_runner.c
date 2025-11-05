// tests/test_runner.c
#include "unity.h"

// prototipos (de lexer_test.c)
void test_lexer_init(void);
void test_token_list_operations(void);
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

// Identificadores
void test_tokenize_simple_identifier(void);
void test_tokenize_identifier_with_underscore(void);
void test_tokenize_identifier_with_numbers(void);
void test_tokenize_identifier_with_numbers_and_underscores(void);

// Palabras reservadas
void test_tokenize_keywords_if(void);
void test_tokenize_keywords_types(void);
void test_tokenize_keywords_loop(void);
void test_tokenize_keywords_functions(void);
void test_tokenize_keywords_io(void);
void test_tokenize_keywords_boolean_literals(void);
void test_tokenize_keywords_logic(void);

// Literales
void test_tokenize_simple_string(void);
void test_tokenize_empty_string_literal(void);
void test_tokenize_simple_char(void);
void test_tokenize_char_escape_sequences(void);
void test_tokenize_char_escape_tab(void);
void test_tokenize_char_escape_backslash(void);
void test_tokenize_string_with_escapes(void);
void test_tokenize_unclosed_string(void);
void test_tokenize_unclosed_char(void);
void test_tokenize_empty_char(void);
void test_tokenize_multichar_invalid(void);
void test_tokenize_invalid_escape_sequence(void);
void test_tokenize_mixed_with_literals(void);
void test_literal_position_with_escapes_and_real_newlines(void);

// Operadores
void test_tokenize_arithmetic_operators(void);
void test_tokenize_power_operator(void);
void test_tokenize_power_vs_multiply(void);
void test_tokenize_comparison_operators(void);
void test_tokenize_logical_operators(void);
void test_tokenize_greater_vs_greater_equal(void);
void test_tokenize_assignment_operator(void);
void test_tokenize_assignment_vs_equality(void);
void test_tokenize_compound_assignment_operators(void);
void test_tokenize_operators_without_spaces(void);
void test_tokenize_expression_with_operators(void);
void test_operators_position_tracking(void);

// Delimitadores
void test_tokenize_single_delimiters_in_order(void);
void test_tokenize_single_delimiters_no_spaces(void);
void test_delimiters_position_tracking_multiline(void);
void test_mixed_identifiers_numbers_and_delimiters(void);
void test_delimiters_not_confused_with_operators(void);

void register_lexer_tests(void) {
    // Básicos
    RUN_TEST(test_lexer_init);
    RUN_TEST(test_token_list_operations);

    // Identificadores
    RUN_TEST(test_tokenize_simple_identifier);
    RUN_TEST(test_tokenize_identifier_with_underscore);
    RUN_TEST(test_tokenize_identifier_with_numbers);
    RUN_TEST(test_tokenize_identifier_with_numbers_and_underscores);

    // Palabras reservadas
    RUN_TEST(test_tokenize_keywords_if);
    RUN_TEST(test_tokenize_keywords_types);
    RUN_TEST(test_tokenize_keywords_loop);
    RUN_TEST(test_tokenize_keywords_functions);
    RUN_TEST(test_tokenize_keywords_io);
    RUN_TEST(test_tokenize_keywords_boolean_literals);
    RUN_TEST(test_tokenize_keywords_logic);

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

    // Literales
    RUN_TEST(test_tokenize_simple_string);
    RUN_TEST(test_tokenize_empty_string_literal);
    RUN_TEST(test_tokenize_simple_char);
    RUN_TEST(test_tokenize_char_escape_sequences);
    RUN_TEST(test_tokenize_char_escape_tab);
    RUN_TEST(test_tokenize_char_escape_backslash);
    RUN_TEST(test_tokenize_string_with_escapes);

    // Errores en literales
    RUN_TEST(test_tokenize_unclosed_string);
    RUN_TEST(test_tokenize_unclosed_char);
    RUN_TEST(test_tokenize_empty_char);
    RUN_TEST(test_tokenize_multichar_invalid);
    RUN_TEST(test_tokenize_invalid_escape_sequence);
    RUN_TEST(test_literal_position_with_escapes_and_real_newlines);

    // Mixtos
    RUN_TEST(test_tokenize_mixed_with_literals);

    // Operadores
    RUN_TEST(test_tokenize_arithmetic_operators);
    RUN_TEST(test_tokenize_power_operator);
    RUN_TEST(test_tokenize_power_vs_multiply);
    RUN_TEST(test_tokenize_comparison_operators);
    RUN_TEST(test_tokenize_logical_operators);
    RUN_TEST(test_tokenize_greater_vs_greater_equal);
    RUN_TEST(test_tokenize_assignment_operator);
    RUN_TEST(test_tokenize_assignment_vs_equality);
    RUN_TEST(test_tokenize_compound_assignment_operators);
    RUN_TEST(test_tokenize_operators_without_spaces);
    RUN_TEST(test_tokenize_expression_with_operators);
    RUN_TEST(test_operators_position_tracking);
    
    // Delimitadores
    RUN_TEST(test_tokenize_single_delimiters_in_order);
    RUN_TEST(test_tokenize_single_delimiters_no_spaces);
    RUN_TEST(test_delimiters_position_tracking_multiline);
    RUN_TEST(test_mixed_identifiers_numbers_and_delimiters);
    RUN_TEST(test_delimiters_not_confused_with_operators);
}
