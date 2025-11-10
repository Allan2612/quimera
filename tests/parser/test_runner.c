// tests/parser/test_runner.c
#include "unity.h"

// Prototipos de tests de parser
void test_parser_simple_variable_declaration(void);
void test_parser_variable_with_initialization(void);
void test_parser_array_declaration(void);
void test_parser_list_declaration(void);
void test_parser_function_declaration(void);
void test_parser_function_with_parameters(void);
void test_parser_function_with_return_type_null(void);

// Tests de expresiones
void test_parser_arithmetic_expressions(void);
void test_parser_logical_expressions(void);
void test_parser_comparison_expressions(void);
void test_parser_function_calls(void);
void test_parser_array_indexing(void);
void test_parser_list_methods(void);
void test_parser_precedence(void);
void test_parser_parentheses(void);
void test_parser_unary_operators(void);
void test_parser_assignment_operators(void);

// Tests de sentencias
void test_parser_if_statement(void);
void test_parser_if_else_statement(void);
void test_parser_if_elseif_else_statement(void);
void test_parser_while_statement(void);
void test_parser_for_statement(void);
void test_parser_print_statement(void);
void test_parser_read_statement(void);
void test_parser_return_statement(void);

// Tests de bloques y programas
void test_parser_block_with_multiple_statements(void);
void test_parser_nested_blocks(void);
void test_parser_complete_program(void);

// Tests de errores
void test_parser_error_handling_missing_semicolon(void);
void test_parser_error_handling_missing_end(void);
void test_parser_error_handling_unexpected_token(void);
void test_parser_error_recovery(void);

void register_parser_tests(void) {
    // Tests de declaraciones
    RUN_TEST(test_parser_simple_variable_declaration);
    RUN_TEST(test_parser_variable_with_initialization);
    RUN_TEST(test_parser_array_declaration);
    RUN_TEST(test_parser_list_declaration);
    RUN_TEST(test_parser_function_declaration);
    RUN_TEST(test_parser_function_with_parameters);
    RUN_TEST(test_parser_function_with_return_type_null);

    // Tests de expresiones
    RUN_TEST(test_parser_arithmetic_expressions);
    RUN_TEST(test_parser_logical_expressions);
    RUN_TEST(test_parser_comparison_expressions);
    RUN_TEST(test_parser_function_calls);
    RUN_TEST(test_parser_array_indexing);
    RUN_TEST(test_parser_list_methods);
    RUN_TEST(test_parser_precedence);
    RUN_TEST(test_parser_parentheses);
    RUN_TEST(test_parser_unary_operators);
    RUN_TEST(test_parser_assignment_operators);

    // Tests de sentencias
    RUN_TEST(test_parser_if_statement);
    RUN_TEST(test_parser_if_else_statement);
    RUN_TEST(test_parser_if_elseif_else_statement);
    RUN_TEST(test_parser_while_statement);
    RUN_TEST(test_parser_for_statement);
    RUN_TEST(test_parser_print_statement);
    RUN_TEST(test_parser_read_statement);
    RUN_TEST(test_parser_return_statement);

    // Tests de bloques y programas
    RUN_TEST(test_parser_block_with_multiple_statements);
    RUN_TEST(test_parser_nested_blocks);
    RUN_TEST(test_parser_complete_program);

    // Tests de errores
    RUN_TEST(test_parser_error_handling_missing_semicolon);
    RUN_TEST(test_parser_error_handling_missing_end);
    RUN_TEST(test_parser_error_handling_unexpected_token);
    RUN_TEST(test_parser_error_recovery);
}