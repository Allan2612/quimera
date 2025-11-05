// tests/lexer/delimiters_tests.c
#include "unity.h"
#include "lexer.h"
#include "utils.h"
#include <string.h>

// ====== TESTS BÁSICOS ======

void test_tokenize_single_delimiters_in_order(void) {
    Lexer lexer;
    TokenList list;
    // Mismo orden que tu tabla: ; : { } , ( ) [ ]
    const char *source = "; : { } , ( ) [ ]";

    init_lexer(&lexer, source);
    init_token_list(&list);
    tokenize(&lexer, &list);

    TEST_ASSERT_EQUAL(9, list.count);
    const char *expect[] = { ";", ":", "{", "}", ",", "(", ")", "[", "]" };
    for (int i = 0; i < 9; ++i) {
        TEST_ASSERT_EQUAL(TOK_DELIMITER, list.tokens[i].type);
        TEST_ASSERT_EQUAL_STRING_LEN(expect[i], list.tokens[i].lexeme, 1);
        TEST_ASSERT_EQUAL(1, list.tokens[i].length);
    }

    free_token_list(&list);
}

void test_tokenize_single_delimiters_no_spaces(void) {
    Lexer lexer;
    TokenList list;
    const char *source = ";:{}(),[]";

    init_lexer(&lexer, source);
    init_token_list(&list);
    tokenize(&lexer, &list);

    TEST_ASSERT_EQUAL(9, list.count);
    const char *expect[] = { ";", ":", "{", "}", "(", ")", ",", "[", "]" };
    for (int i = 0; i < 9; ++i) {
        TEST_ASSERT_EQUAL(TOK_DELIMITER, list.tokens[i].type);
        TEST_ASSERT_EQUAL_STRING_LEN(expect[i], list.tokens[i].lexeme, 1);
        TEST_ASSERT_EQUAL(1, list.tokens[i].length);
    }

    free_token_list(&list);
}

// ====== TESTS DE POSICIÓN ======

void test_delimiters_position_tracking_multiline(void) {
    Lexer lexer;
    TokenList list;
    // línea 1: "{"
    // línea 2: "(a,b);"
    // línea 3: "}"
    const char *source = "{\n(a,b);\n}";

    init_lexer(&lexer, source);
    init_token_list(&list);
    tokenize(&lexer, &list);

    // Tokens esperados (solo delimitadores + identificadores/números relevantes):
    // {  (  a  ,  b  )  ;  }
    // 0  1  2  3  4  5  6  7  -> total 8
    TEST_ASSERT_EQUAL(8, list.count);

    // {  en línea 1, columna 1
    TEST_ASSERT_EQUAL(TOK_DELIMITER, list.tokens[0].type);
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);
    TEST_ASSERT_EQUAL(1, list.tokens[0].column);

    // ( en línea 2, columna 1
    TEST_ASSERT_EQUAL(TOK_DELIMITER, list.tokens[1].type);
    TEST_ASSERT_EQUAL(2, list.tokens[1].line);
    TEST_ASSERT_EQUAL(1, list.tokens[1].column);

    // a en línea 2, columna 3
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[2].type);
    TEST_ASSERT_EQUAL(2, list.tokens[2].line);
    TEST_ASSERT_EQUAL(2, list.tokens[2].column);

    // , en línea 2, columna 3 (después de 'a')
    TEST_ASSERT_EQUAL(TOK_DELIMITER, list.tokens[3].type);
    TEST_ASSERT_EQUAL(2, list.tokens[3].line);
    TEST_ASSERT_EQUAL(3, list.tokens[3].column);

    // b en línea 2, columna 3
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[4].type);
    TEST_ASSERT_EQUAL(2, list.tokens[4].line);
    TEST_ASSERT_EQUAL(4, list.tokens[4].column);

    // ) en línea 2, columna 5 (después de 'b')
    TEST_ASSERT_EQUAL(TOK_DELIMITER, list.tokens[5].type);
    TEST_ASSERT_EQUAL(2, list.tokens[5].line);
    TEST_ASSERT_EQUAL(5, list.tokens[5].column);

    // ; en línea 2, columna 6
    TEST_ASSERT_EQUAL(TOK_DELIMITER, list.tokens[6].type);
    TEST_ASSERT_EQUAL(2, list.tokens[6].line);
    TEST_ASSERT_EQUAL(6, list.tokens[6].column);

    // } en línea 3, columna 1
    TEST_ASSERT_EQUAL(TOK_DELIMITER, list.tokens[7].type);
    TEST_ASSERT_EQUAL(3, list.tokens[7].line);
    TEST_ASSERT_EQUAL(1, list.tokens[7].column);

    free_token_list(&list);
}
