#include "unity.h"
#include "analyzer.h"

// Forward declarations de todos los tests
void test_analyzer_valid_integer_declaration(void);
void test_analyzer_valid_float_declaration(void);
void test_analyzer_valid_string_declaration(void);
void test_analyzer_valid_char_declaration(void);
void test_analyzer_valid_boolean_declaration(void);
void test_analyzer_redeclaration_error(void);
void test_analyzer_use_before_declaration(void);
void test_analyzer_type_mismatch_assignment(void);
void test_analyzer_integer_to_float_conversion(void);
void test_analyzer_char_to_string_conversion(void);
void test_analyzer_invalid_float_to_integer_conversion(void);
void test_analyzer_invalid_boolean_arithmetic(void);
void test_analyzer_valid_arithmetic_operations(void);
void test_analyzer_valid_logical_operations(void);
void test_analyzer_scope_variable_shadowing(void);
void test_analyzer_scope_nested_blocks(void);

void test_analyzer_comparison_compatible_types(void);
void test_analyzer_char_string_equality_valid(void);
void test_analyzer_comparison_incompatible_types(void);
void test_analyzer_boolean_integer_comparison_error(void);
void test_analyzer_if_boolean_condition_valid(void);
void test_analyzer_if_non_boolean_condition_error(void);
void test_analyzer_while_boolean_condition_valid(void);
void test_analyzer_while_non_boolean_condition_error(void);
void test_analyzer_for_boolean_condition_valid(void);
void test_analyzer_for_non_boolean_condition_error(void);
void test_analyzer_valid_assignment(void);
void test_analyzer_invalid_assignment(void);
void test_analyzer_compound_assignment_valid(void);
void test_analyzer_compound_assignment_invalid(void);
void test_analyzer_assignment_to_non_lvalue(void);

void test_analyzer_function_declaration_valid(void);
void test_analyzer_function_no_parameters_valid(void);
void test_analyzer_function_multiple_params_valid(void);
void test_analyzer_function_wrong_return_type(void);
void test_analyzer_void_function_returns_value(void);
void test_analyzer_non_void_function_no_return(void);
void test_analyzer_function_call_valid(void);
void test_analyzer_function_call_wrong_arg_count(void);
void test_analyzer_function_call_wrong_arg_types(void);
void test_analyzer_function_call_undeclared(void);
void test_analyzer_function_call_auto_conversion(void);
void test_analyzer_recursive_function_valid(void);
void test_analyzer_function_overload_valid(void);
void test_analyzer_parameter_scope_valid(void);
void test_analyzer_parameter_scope_invalid(void);

void test_analyzer_arithmetic_expression_valid(void);
void test_analyzer_arithmetic_auto_conversion_valid(void);
void test_analyzer_arithmetic_incompatible_types(void);
void test_analyzer_division_valid_types(void);
void test_analyzer_division_by_zero(void);
void test_analyzer_logical_expression_valid(void);
void test_analyzer_logical_non_boolean_operands(void);
void test_analyzer_negation_non_boolean(void);
void test_analyzer_relational_expressions_valid(void);
void test_analyzer_comparison_auto_conversion(void);
void test_analyzer_comparison_incompatible_types_expr(void);
void test_analyzer_mixed_expression_valid(void);
void test_analyzer_nested_expression_valid(void);
void test_analyzer_unary_minus_valid(void);
void test_analyzer_unary_minus_invalid_type(void);
void test_analyzer_operator_precedence_arithmetic(void);
void test_analyzer_operator_precedence_logical(void);

// Test runner principal
int run_all_analyzer_tests(void) {
    UNITY_BEGIN();

    // Tests de tipos y declaraciones
    RUN_TEST(test_analyzer_valid_integer_declaration);
    RUN_TEST(test_analyzer_valid_float_declaration);
    RUN_TEST(test_analyzer_valid_string_declaration);
    RUN_TEST(test_analyzer_valid_char_declaration);
    RUN_TEST(test_analyzer_valid_boolean_declaration);
    RUN_TEST(test_analyzer_redeclaration_error);
    RUN_TEST(test_analyzer_use_before_declaration);
    RUN_TEST(test_analyzer_type_mismatch_assignment);
    RUN_TEST(test_analyzer_integer_to_float_conversion);
    RUN_TEST(test_analyzer_char_to_string_conversion);
    RUN_TEST(test_analyzer_invalid_float_to_integer_conversion);
    RUN_TEST(test_analyzer_invalid_boolean_arithmetic);
    RUN_TEST(test_analyzer_valid_arithmetic_operations);
    RUN_TEST(test_analyzer_valid_logical_operations);
    RUN_TEST(test_analyzer_scope_variable_shadowing);
    RUN_TEST(test_analyzer_scope_nested_blocks);

    // Tests de flujo de control
    RUN_TEST(test_analyzer_comparison_compatible_types);
    RUN_TEST(test_analyzer_char_string_equality_valid);
    RUN_TEST(test_analyzer_comparison_incompatible_types);
    RUN_TEST(test_analyzer_boolean_integer_comparison_error);
    RUN_TEST(test_analyzer_if_boolean_condition_valid);
    RUN_TEST(test_analyzer_if_non_boolean_condition_error);
    RUN_TEST(test_analyzer_while_boolean_condition_valid);
    RUN_TEST(test_analyzer_while_non_boolean_condition_error);
    RUN_TEST(test_analyzer_for_boolean_condition_valid);
    RUN_TEST(test_analyzer_for_non_boolean_condition_error);
    RUN_TEST(test_analyzer_valid_assignment);
    RUN_TEST(test_analyzer_invalid_assignment);
    RUN_TEST(test_analyzer_compound_assignment_valid);
    RUN_TEST(test_analyzer_compound_assignment_invalid);
    RUN_TEST(test_analyzer_assignment_to_non_lvalue);

    // Tests de funciones
    RUN_TEST(test_analyzer_function_declaration_valid);
    RUN_TEST(test_analyzer_function_no_parameters_valid);
    RUN_TEST(test_analyzer_function_multiple_params_valid);
    RUN_TEST(test_analyzer_function_wrong_return_type);
    RUN_TEST(test_analyzer_void_function_returns_value);
    RUN_TEST(test_analyzer_non_void_function_no_return);
    RUN_TEST(test_analyzer_function_call_valid);
    RUN_TEST(test_analyzer_function_call_wrong_arg_count);
    RUN_TEST(test_analyzer_function_call_wrong_arg_types);
    RUN_TEST(test_analyzer_function_call_undeclared);
    RUN_TEST(test_analyzer_function_call_auto_conversion);
    RUN_TEST(test_analyzer_recursive_function_valid);
    RUN_TEST(test_analyzer_function_overload_valid);
    RUN_TEST(test_analyzer_parameter_scope_valid);
    RUN_TEST(test_analyzer_parameter_scope_invalid);

    // Tests de expresiones
    RUN_TEST(test_analyzer_arithmetic_expression_valid);
    RUN_TEST(test_analyzer_arithmetic_auto_conversion_valid);
    RUN_TEST(test_analyzer_arithmetic_incompatible_types);
    RUN_TEST(test_analyzer_division_valid_types);
    RUN_TEST(test_analyzer_division_by_zero);
    RUN_TEST(test_analyzer_logical_expression_valid);
    RUN_TEST(test_analyzer_logical_non_boolean_operands);
    RUN_TEST(test_analyzer_negation_non_boolean);
    RUN_TEST(test_analyzer_relational_expressions_valid);
    RUN_TEST(test_analyzer_comparison_auto_conversion);
    RUN_TEST(test_analyzer_comparison_incompatible_types_expr);
    RUN_TEST(test_analyzer_mixed_expression_valid);
    RUN_TEST(test_analyzer_nested_expression_valid);
    RUN_TEST(test_analyzer_unary_minus_valid);
    RUN_TEST(test_analyzer_unary_minus_invalid_type);
    RUN_TEST(test_analyzer_operator_precedence_arithmetic);
    RUN_TEST(test_analyzer_operator_precedence_logical);

    return UNITY_END();
}