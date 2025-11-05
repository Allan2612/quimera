#include "unity.h"
#include "lexer.h"
#include "utils.h"
#include "diag_errors.h"
#include <stdio.h>
#include <string.h>

// ===== TESTS DE NÚMEROS =====

void test_tokenize_integer(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "123";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_INTEGER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("123", list.tokens[0].lexeme, 3);
    TEST_ASSERT_EQUAL(3, list.tokens[0].length);
    
    free_token_list(&list);
}

void test_tokenize_float_number(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "123.45";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_FLOAT, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("123.45", list.tokens[0].lexeme, 6);
    TEST_ASSERT_EQUAL(6, list.tokens[0].length);
    
    free_token_list(&list);
}

void test_tokenize_invalid_double_dot_number(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "123.45.67";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ERROR, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("123.45.67", list.tokens[0].lexeme, 9);
    
    free_token_list(&list);
}

void test_tokenize_zero(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "0";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_INTEGER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("0", list.tokens[0].lexeme, 1);
    
    free_token_list(&list);
}