#include "unity.h"
#include "lexer.h"
#include "utils.h"
#include "diag_errors.h"
#include <stdio.h>
#include <string.h>

void setUp(void)   { /* vacío a propósito */ }
void tearDown(void){ /* vacío a propósito */ }

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

// ===== TESTS DE LITERALES (STRINGS Y CHARS) =====

void test_tokenize_simple_string(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "\"hello world\"";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("hello world", list.tokens[0].lexeme, 11);
    TEST_ASSERT_EQUAL(11, list.tokens[0].length);
    
    free_token_list(&list);
}

void test_tokenize_empty_string_literal(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "\"\"";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[0].type);
    TEST_ASSERT_EQUAL(0, list.tokens[0].length);
    
    free_token_list(&list);
}

void test_tokenize_simple_char(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "'a'";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_CHAR, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("a", list.tokens[0].lexeme, 1);
    TEST_ASSERT_EQUAL(1, list.tokens[0].length);
    
    free_token_list(&list);
}

void test_tokenize_char_escape_sequences(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "'\\n'";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_CHAR, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("\\n", list.tokens[0].lexeme, 2);
    
    free_token_list(&list);
}

void test_tokenize_char_escape_tab(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "'\\t'";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_CHAR, list.tokens[0].type);
    
    free_token_list(&list);
}

void test_tokenize_char_escape_backslash(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "'\\\\'";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_CHAR, list.tokens[0].type);
    
    free_token_list(&list);
}

void test_tokenize_string_with_escapes(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "\"Hello\\nWorld\"";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("Hello\\nWorld", list.tokens[0].lexeme, 12);
    
    free_token_list(&list);
}

// ===== TESTS DE ERRORES EN LITERALES =====

void test_tokenize_unclosed_string(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "\"unclosed string";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ERROR, list.tokens[0].type);
    
    free_token_list(&list);
}

void test_tokenize_unclosed_char(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "'unclosed";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ERROR, list.tokens[0].type);
    
    free_token_list(&list);
}

void test_tokenize_empty_char(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "''";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ERROR, list.tokens[0].type);
    
    free_token_list(&list);
}

void test_tokenize_multichar_invalid(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "'abc'";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ERROR, list.tokens[0].type);
    
    free_token_list(&list);
}

void test_tokenize_invalid_escape_sequence(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "'\\x'";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ERROR, list.tokens[0].type);
    
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