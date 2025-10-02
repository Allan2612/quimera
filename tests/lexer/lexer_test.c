#include "unity.h"
#include "lexer.h"
#include "utils.h"
#include "diag_errors.h"
#include <stdio.h>
#include <string.h>

// ===== TESTS DE FUNCIONES HELPER =====

void test_lexer_init(void) {
    Lexer lexer;
    const char *source = "test input";

    init_lexer(&lexer, source);

    TEST_ASSERT_EQUAL_PTR(source, lexer.source);
    TEST_ASSERT_EQUAL_INT((int)strlen(source), lexer.length);  // <-- 10
    TEST_ASSERT_EQUAL_INT(0, lexer.position);
    TEST_ASSERT_EQUAL_INT(1, lexer.line);
    TEST_ASSERT_EQUAL_INT(1, lexer.column);
}

void test_token_list_operations(void) {
    TokenList list;
    init_token_list(&list);
    
    TEST_ASSERT_NOT_NULL(list.tokens);
    TEST_ASSERT_EQUAL(0, list.count);
    TEST_ASSERT_EQUAL(16, list.capacity);  // TOKEN_LIST_INITIAL_CAPACITY
    
    free_token_list(&list);
}

// ===== TESTS DE MÚLTIPLES TOKENS =====

void test_tokenize_multiple_tokens(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "If variable 123";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(3, list.count);
    
    // Token 1: "If"
    TEST_ASSERT_EQUAL(TOK_KEYWORD, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("If", list.tokens[0].lexeme, 2);
    
    // Token 2: "variable"  
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[1].type);
    TEST_ASSERT_EQUAL_STRING_LEN("variable", list.tokens[1].lexeme, 8);
    
    // Token 3: "123"
    TEST_ASSERT_EQUAL(TOK_INTEGER, list.tokens[2].type);
    TEST_ASSERT_EQUAL_STRING_LEN("123", list.tokens[2].lexeme, 3);
    
    free_token_list(&list);
}

void test_tokenize_identifier_and_number_mixed(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "count 42 name";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(3, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL(TOK_INTEGER, list.tokens[1].type);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[2].type);
    
    free_token_list(&list);
}

// ===== TESTS DE MANEJO DE ESPACIOS =====

void test_tokenize_with_spaces(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "  variable   123  ";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(2, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL(TOK_INTEGER, list.tokens[1].type);
    
    free_token_list(&list);
}

void test_tokenize_with_tabs_and_newlines(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "var1\t\nvar2";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(2, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("var1", list.tokens[0].lexeme, 4);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[1].type);
    TEST_ASSERT_EQUAL_STRING_LEN("var2", list.tokens[1].lexeme, 4);
    
    free_token_list(&list);
}

// ===== TESTS DE CASOS EDGE =====

void test_tokenize_empty_string(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(0, list.count);
    
    free_token_list(&list);
}

void test_tokenize_only_spaces(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "   \t\n  ";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(0, list.count);
    
    free_token_list(&list);
}

void test_tokenize_single_character_identifier(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "a";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("a", list.tokens[0].lexeme, 1);
    
    free_token_list(&list);
}

// ===== TESTS DE POSICIÓN Y LÍNEAS =====

void test_lexer_position_tracking(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "var1\nvar2";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(2, list.count);
    
    // Primer token en línea 1
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);
    TEST_ASSERT_EQUAL(1, list.tokens[0].column);
    
    // Segundo token en línea 2
    TEST_ASSERT_EQUAL(2, list.tokens[1].line);
    TEST_ASSERT_EQUAL(1, list.tokens[1].column);
    
    free_token_list(&list);
}

// ===== TESTS MIXTOS CON LITERALES =====

void test_tokenize_mixed_with_literals(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "variable \"string\" 'c' 123";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(4, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[1].type);
    TEST_ASSERT_EQUAL(TOK_CHAR, list.tokens[2].type);
    TEST_ASSERT_EQUAL(TOK_INTEGER, list.tokens[3].type);
    
    free_token_list(&list);
}