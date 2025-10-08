#include "unity.h"
#include "lexer.h"
#include "utils.h"
#include "diag_errors.h"
#include <stdio.h>
#include <string.h>

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

void test_literal_position_with_escapes_and_real_newlines(void) {
    Lexer lexer;
    TokenList list;

    // Caso 1: el literal contiene una secuencia de escape "\n" -> NO avanza línea
    const char *source1 = "\"Hola\\nMundo\"";
    init_lexer(&lexer, source1);
    init_token_list(&list);
    tokenize(&lexer, &list);

    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[0].type);
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);   // sigue en la misma línea
    TEST_ASSERT_EQUAL(1, list.tokens[0].column); // inicia en col 1
    free_token_list(&list);

    // Caso 2: el literal incluye un salto de línea real dentro del texto fuente
    const char *source2 = "\"Hola\nMundo\""; 
    init_lexer(&lexer, source2);
    init_token_list(&list);
    tokenize(&lexer, &list);

    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[0].type);
    // Debe haber incrementado la línea dentro del literal
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);     // empieza en línea 1
    TEST_ASSERT_TRUE(lexer.line > 1);              // el lexer ahora en línea 2
    TEST_ASSERT_EQUAL(1, list.tokens[0].column);   // columna inicial sigue siendo 1
    free_token_list(&list);

    // Caso 3 (combinado): literal con escape y luego salto real
    const char *source3 = "\"Linea1\\n\nLinea2\"";
    init_lexer(&lexer, source3);
    init_token_list(&list);
    tokenize(&lexer, &list);

    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[0].type);
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);
    TEST_ASSERT_TRUE(lexer.line > 1); // debe haberse avanzado de línea real
    free_token_list(&list);
}