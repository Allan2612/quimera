#include "unity.h"
#include "lexer.h"
#include "utils.h"
#include "diag_errors.h"
#include <stdio.h>
#include <string.h>

// ===== TESTS DE OPERADORES ARITMÉTICOS =====

void test_tokenize_arithmetic_operators(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "+ - * / %";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(5, list.count);
    
    // Verificar cada operador
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("+", list.tokens[0].lexeme, 1);
    
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP, list.tokens[1].type);
    TEST_ASSERT_EQUAL_STRING_LEN("-", list.tokens[1].lexeme, 1);
    
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP, list.tokens[2].type);
    TEST_ASSERT_EQUAL_STRING_LEN("*", list.tokens[2].lexeme, 1);
    
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP, list.tokens[3].type);
    TEST_ASSERT_EQUAL_STRING_LEN("/", list.tokens[3].lexeme, 1);
    
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP, list.tokens[4].type);
    TEST_ASSERT_EQUAL_STRING_LEN("%", list.tokens[4].lexeme, 1);
    
    free_token_list(&list);
}

void test_tokenize_power_operator(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "**";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("**", list.tokens[0].lexeme, 2);
    TEST_ASSERT_EQUAL(2, list.tokens[0].length);
    
    free_token_list(&list);
}

void test_tokenize_power_vs_multiply(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "* **";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(2, list.count);
    
    // Primer token: "*"
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("*", list.tokens[0].lexeme, 1);
    TEST_ASSERT_EQUAL(1, list.tokens[0].length);
    
    // Segundo token: "**"
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP, list.tokens[1].type);
    TEST_ASSERT_EQUAL_STRING_LEN("**", list.tokens[1].lexeme, 2);
    TEST_ASSERT_EQUAL(2, list.tokens[1].length);
    
    free_token_list(&list);
}

// ===== TESTS DE OPERADORES LÓGICOS/COMPARACIÓN =====

void test_tokenize_comparison_operators(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "== != > < >= <=";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(6, list.count);
    
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("==", list.tokens[0].lexeme, 2);
    
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[1].type);
    TEST_ASSERT_EQUAL_STRING_LEN("!=", list.tokens[1].lexeme, 2);
    
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[2].type);
    TEST_ASSERT_EQUAL_STRING_LEN(">", list.tokens[2].lexeme, 1);
    
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[3].type);
    TEST_ASSERT_EQUAL_STRING_LEN("<", list.tokens[3].lexeme, 1);
    
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[4].type);
    TEST_ASSERT_EQUAL_STRING_LEN(">=", list.tokens[4].lexeme, 2);
    
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[5].type);
    TEST_ASSERT_EQUAL_STRING_LEN("<=", list.tokens[5].lexeme, 2);
    
    free_token_list(&list);
}

void test_tokenize_logical_operators(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "&& || !";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(3, list.count);
    
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("&&", list.tokens[0].lexeme, 2);
    
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[1].type);
    TEST_ASSERT_EQUAL_STRING_LEN("||", list.tokens[1].lexeme, 2);
    
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[2].type);
    TEST_ASSERT_EQUAL_STRING_LEN("!", list.tokens[2].lexeme, 1);
    
    free_token_list(&list);
}

void test_tokenize_greater_vs_greater_equal(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "> >=";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(2, list.count);
    
    // Primer token: ">"
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN(">", list.tokens[0].lexeme, 1);
    TEST_ASSERT_EQUAL(1, list.tokens[0].length);
    
    // Segundo token: ">="
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[1].type);
    TEST_ASSERT_EQUAL_STRING_LEN(">=", list.tokens[1].lexeme, 2);
    TEST_ASSERT_EQUAL(2, list.tokens[1].length);
    
    free_token_list(&list);
}

// ===== TESTS DE OPERADORES DE ASIGNACIÓN =====

void test_tokenize_assignment_operator(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "=";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ASSIGNMENT_OP, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("=", list.tokens[0].lexeme, 1);
    TEST_ASSERT_EQUAL(1, list.tokens[0].length);
    
    free_token_list(&list);
}

void test_tokenize_assignment_vs_equality(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "= ==";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(2, list.count);
    
    // Primer token: "=" (asignación)
    TEST_ASSERT_EQUAL(TOK_ASSIGNMENT_OP, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("=", list.tokens[0].lexeme, 1);
    TEST_ASSERT_EQUAL(1, list.tokens[0].length);
    
    // Segundo token: "==" (comparación)
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[1].type);
    TEST_ASSERT_EQUAL_STRING_LEN("==", list.tokens[1].lexeme, 2);
    TEST_ASSERT_EQUAL(2, list.tokens[1].length);
    
    free_token_list(&list);
}

// ===== TESTS MIXTOS Y CASOS COMPLEJOS =====

void test_tokenize_operators_without_spaces(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "a+b*c>=d";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(7, list.count);
    
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);      // a
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP, list.tokens[1].type);   // +
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[2].type);      // b
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP, list.tokens[3].type);   // *
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[4].type);      // c
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP, list.tokens[5].type);        // >=
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[6].type);      // d
    
    free_token_list(&list);
}

void test_tokenize_expression_with_operators(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "x = y + z * 2";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(7, list.count);
    
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);      // x
    TEST_ASSERT_EQUAL(TOK_ASSIGNMENT_OP, list.tokens[1].type);   // =
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[2].type);      // y
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP, list.tokens[3].type);   // +
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[4].type);      // z
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP, list.tokens[5].type);   // *
    TEST_ASSERT_EQUAL(TOK_INTEGER, list.tokens[6].type);         // 2
    
    free_token_list(&list);
}

// ===== TESTS DE POSICIONAMIENTO =====

void test_operators_position_tracking(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "+ ==\n<=";
    
    init_lexer(&lexer, source);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(3, list.count);
    
    // Primer operador "+" en línea 1, columna 1
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);
    TEST_ASSERT_EQUAL(1, list.tokens[0].column);
    
    // Segundo operador "==" en línea 1, columna 3
    TEST_ASSERT_EQUAL(1, list.tokens[1].line);
    TEST_ASSERT_EQUAL(3, list.tokens[1].column);
    
    // Tercer operador "<=" en línea 2, columna 1
    TEST_ASSERT_EQUAL(2, list.tokens[2].line);
    TEST_ASSERT_EQUAL(1, list.tokens[2].column);
    
    free_token_list(&list);
}

