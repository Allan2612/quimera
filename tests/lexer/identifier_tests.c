#include "unity.h"
#include "lexer.h"
#include "utils.h"
#include "diag_errors.h"
#include <stdio.h>
#include <string.h>

// ===== TESTS DE IDENTIFICADORES =====

void test_tokenize_simple_identifier(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "variable";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("variable", list.tokens[0].lexeme, 8);
    TEST_ASSERT_EQUAL(8, list.tokens[0].length);
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);
    TEST_ASSERT_EQUAL(1, list.tokens[0].column);
    
    free_token_list(&list);
}

void test_tokenize_identifier_with_underscore(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "private_var";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("private_var", list.tokens[0].lexeme, 11);
    TEST_ASSERT_EQUAL(11, list.tokens[0].length);
    
    free_token_list(&list);
}

void test_tokenize_identifier_with_numbers(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "var123test";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("var123test", list.tokens[0].lexeme, 10);
    
    free_token_list(&list);
}

// ===== TESTS DE PALABRAS RESERVADAS =====

void test_tokenize_keyword_if(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "If";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_KEYWORD, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("If", list.tokens[0].lexeme, 2);
    
    free_token_list(&list);
}

void test_tokenize_keyword_else(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "Else";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_KEYWORD, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("Else", list.tokens[0].lexeme, 4);
    
    free_token_list(&list);
}