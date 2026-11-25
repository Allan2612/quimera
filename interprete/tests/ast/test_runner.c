// tests/test_runner.c
#include "unity.h"

void test_arena_alloc_and_ast_new(void);
void test_ast_list_push_growth_preserves_order(void);
void test_build_binop_tree_precedence_add_mul(void);
void test_if_with_elseifs_and_else_structure(void);
void test_assign_node_right_associative_shape(void);
void test_call_and_index_nodes(void);
void test_literal_variants_int_float_string_bool_null(void);

void register_ast_tests(void) { 
    RUN_TEST(test_arena_alloc_and_ast_new);
    RUN_TEST(test_ast_list_push_growth_preserves_order);
    RUN_TEST(test_build_binop_tree_precedence_add_mul);
    RUN_TEST(test_if_with_elseifs_and_else_structure);
    RUN_TEST(test_assign_node_right_associative_shape);
    RUN_TEST(test_call_and_index_nodes);
    RUN_TEST(test_literal_variants_int_float_string_bool_null);
}
