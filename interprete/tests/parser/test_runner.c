// tests/parser/test_runner.c
#include "unity.h"

/* ================ Prototipos de Tests de Error Handling ================ */
// error_handling_tests.c
void test_parser_if_missing_end(void);
void test_parser_while_missing_end(void);
void test_parser_for_missing_end(void);
void test_parser_if_missing_parenthesis(void);
void test_parser_declaration_missing_type(void);
void test_parser_unexpected_token(void);
void test_parser_error_recovery_multiple(void);
void test_parser_unexpected_eof(void);
void test_parser_nested_blocks_with_errors(void);

/* ================ Prototipos de Tests de Declaraciones ================ */
// declaration_tests.c
void test_parser_var_declaration_integer(void);
void test_parser_var_declaration_with_init(void);
void test_parser_var_declaration_float(void);
void test_parser_var_declaration_string(void);
void test_parser_var_declaration_boolean(void);
void test_parser_var_declaration_char(void);
void test_parser_array_declaration_integer(void);
void test_parser_array_declaration_string(void);
void test_parser_list_declaration_integer(void);
void test_parser_list_declaration_string(void);
void test_parser_function_declaration_simple(void);
void test_parser_function_declaration_with_params(void);
void test_parser_multiple_declarations(void);

/* ================ Prototipos de Tests de Control de Flujo ================ */
// control_flow_tests.c
void test_parser_if_simple(void);
void test_parser_if_else(void);
void test_parser_if_elseif_else(void);
void test_parser_if_nested(void);
void test_parser_while_simple(void);
void test_parser_while_nested(void);
void test_parser_for_with_declaration(void);
void test_parser_for_with_assignment(void);
void test_parser_print_simple(void);
void test_parser_print_variable(void);
void test_parser_read(void);
void test_parser_return_with_expression(void);
void test_parser_return_without_expression(void);
void test_parser_complex_program(void);

/* ================ Tests Existentes (statements_tests.c y parser_tests.c) ================ */
// statements_tests.c - mantener los tests existentes importantes
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

// Tests de errores originales
void test_parser_error_handling_missing_semicolon(void);
void test_parser_error_handling_missing_end(void);
void test_parser_error_handling_unexpected_token(void);
void test_parser_error_recovery(void);

void register_parser_tests(void) {
    printf("=== Ejecutando Tests de Parser - Error Handling ===\n");
    
    // Tests de manejo de errores (NUEVOS - error_handling_tests.c)
    RUN_TEST(test_parser_if_missing_end);
    RUN_TEST(test_parser_while_missing_end);
    RUN_TEST(test_parser_for_missing_end);
    RUN_TEST(test_parser_if_missing_parenthesis);
    RUN_TEST(test_parser_declaration_missing_type);
    RUN_TEST(test_parser_unexpected_token);
    RUN_TEST(test_parser_error_recovery_multiple);
    RUN_TEST(test_parser_unexpected_eof);
    RUN_TEST(test_parser_nested_blocks_with_errors);

    printf("=== Ejecutando Tests de Parser - Declaraciones ===\n");
    
    // Tests de declaraciones (NUEVOS - declaration_tests.c)
    RUN_TEST(test_parser_var_declaration_integer);
    RUN_TEST(test_parser_var_declaration_with_init);
    RUN_TEST(test_parser_var_declaration_float);
    RUN_TEST(test_parser_var_declaration_string);
    RUN_TEST(test_parser_var_declaration_boolean);
    RUN_TEST(test_parser_var_declaration_char);
    RUN_TEST(test_parser_array_declaration_integer);
    RUN_TEST(test_parser_array_declaration_string);
    RUN_TEST(test_parser_list_declaration_integer);
    RUN_TEST(test_parser_list_declaration_string);
    RUN_TEST(test_parser_function_declaration_simple);
    RUN_TEST(test_parser_function_declaration_with_params);
    RUN_TEST(test_parser_multiple_declarations);

    printf("=== Ejecutando Tests de Parser - Control de Flujo ===\n");
    
    // Tests de control de flujo (NUEVOS - control_flow_tests.c)
    RUN_TEST(test_parser_if_simple);
    RUN_TEST(test_parser_if_else);
    RUN_TEST(test_parser_if_elseif_else);
    RUN_TEST(test_parser_if_nested);
    RUN_TEST(test_parser_while_simple);
    RUN_TEST(test_parser_while_nested);
    RUN_TEST(test_parser_for_with_declaration);
    RUN_TEST(test_parser_for_with_assignment);
    RUN_TEST(test_parser_print_simple);
    RUN_TEST(test_parser_print_variable);
    RUN_TEST(test_parser_read);
    RUN_TEST(test_parser_return_with_expression);
    RUN_TEST(test_parser_return_without_expression);
    RUN_TEST(test_parser_complex_program);

    printf("=== Ejecutando Tests de Parser - Existentes ===\n");

    // Tests de declaraciones (EXISTENTES)
    RUN_TEST(test_parser_simple_variable_declaration);
    RUN_TEST(test_parser_variable_with_initialization);
    RUN_TEST(test_parser_array_declaration);
    RUN_TEST(test_parser_list_declaration);
    RUN_TEST(test_parser_function_declaration);
    RUN_TEST(test_parser_function_with_parameters);
    RUN_TEST(test_parser_function_with_return_type_null);

    // Tests de expresiones (EXISTENTES)
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

    // Tests de sentencias (EXISTENTES)
    RUN_TEST(test_parser_if_statement);
    RUN_TEST(test_parser_if_else_statement);
    RUN_TEST(test_parser_if_elseif_else_statement);
    RUN_TEST(test_parser_while_statement);
    RUN_TEST(test_parser_for_statement);
    RUN_TEST(test_parser_print_statement);
    RUN_TEST(test_parser_read_statement);
    RUN_TEST(test_parser_return_statement);

    // Tests de bloques y programas (EXISTENTES)
    RUN_TEST(test_parser_block_with_multiple_statements);
    RUN_TEST(test_parser_nested_blocks);
    RUN_TEST(test_parser_complete_program);

    // Tests de errores originales (EXISTENTES)
    RUN_TEST(test_parser_error_handling_missing_semicolon);
    RUN_TEST(test_parser_error_handling_missing_end);
    RUN_TEST(test_parser_error_handling_unexpected_token);
    RUN_TEST(test_parser_error_recovery);
}