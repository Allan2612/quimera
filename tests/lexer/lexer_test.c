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

// ===== TESTS MIXTOS CON DELIMITADORES =====

void test_mixed_identifiers_numbers_and_delimiters(void) {
    Lexer lexer;
    TokenList list;
    const char *source = "Func(x:Integer, y:Float){Print(x);Print(y);}";

    init_lexer(&lexer, source);
    init_token_list(&list);
    tokenize(&lexer, &list);

    // === Verificación general ===
    // Esperamos delimitadores bien identificados en su orden.
    // Delimitadores esperados en la fuente:
    // (  :  ,  :  )  {  (  )  ;  (  )  ;  }
    const char *expected_delims[] = { "(", ":", ",", ":", ")", "{", "(", ")", ";", "(", ")", ";", "}" };
    size_t expected_delim_count = sizeof(expected_delims) / sizeof(expected_delims[0]);

    size_t found_delims = 0;
    for (size_t i = 0; i < list.count; ++i) {
        const Token *t = &list.tokens[i];
        if (t->type == TOK_DELIMITER) {
            TEST_ASSERT_EQUAL_STRING_LEN(expected_delims[found_delims], t->lexeme, strlen(expected_delims[found_delims]));
            found_delims++;
        }
    }
    TEST_ASSERT_EQUAL(expected_delim_count, found_delims);

    // === Verificación de identificadores y palabras clave ===
    // "Func", "x", "y" deben ser identificadores
    // "Integer", "Float", "Print" son palabras reservadas (TOK_KEYWORD)
    int found_func = 0, found_x = 0, found_y = 0;
    int found_integer_kw = 0, found_float_kw = 0, found_print_kw = 0;

    for (size_t i = 0; i < list.count; ++i) {
        const Token *t = &list.tokens[i];
        if (t->type == TOK_IDENTIFIER) {
            if (strncmp(t->lexeme, "Func", 4) == 0) found_func++;
            if (strncmp(t->lexeme, "x", 1) == 0) found_x++;
            if (strncmp(t->lexeme, "y", 1) == 0) found_y++;
        } else if (t->type == TOK_KEYWORD) {
            if (strncmp(t->lexeme, "Integer", 7) == 0) found_integer_kw++;
            if (strncmp(t->lexeme, "Float", 5) == 0) found_float_kw++;
            if (strncmp(t->lexeme, "Print", 5) == 0) found_print_kw++;
        }
    }

    TEST_ASSERT_TRUE(found_func >= 1);
    TEST_ASSERT_TRUE(found_x >= 1);
    TEST_ASSERT_TRUE(found_y >= 1);
    TEST_ASSERT_TRUE(found_integer_kw >= 1);
    TEST_ASSERT_TRUE(found_float_kw >= 1);
    TEST_ASSERT_TRUE(found_print_kw >= 2); // Hay dos llamadas a Print()

    free_token_list(&list);
}

void test_delimiters_not_confused_with_operators(void) {
    Lexer lexer;
    TokenList list;
    // aquí mezclamos '/', operadores y delimitadores para asegurar clasificación
    const char *source = "a/(b+c); d>=e; f==g, h[i]";

    init_lexer(&lexer, source);
    init_token_list(&list);
    tokenize(&lexer, &list);

    // Solo verificamos algunos puntos:
    // '(' y ')' y ';' y ',' y '[' y ']' deben ser TOK_DELIMITER,
    // '/' debe ser TOK_ARITHMETIC_OP, '>=' y '==' deben ser TOK_LOGIC_OP.
    int found_paren_open = 0, found_paren_close = 0, found_semis = 0, found_comma = 0, found_brack_open = 0, found_brack_close = 0;
    int found_div = 0, found_ge = 0, found_eqeq = 0;

    for (size_t i = 0; i < list.count; ++i) {
        const Token *t = &list.tokens[i];
        if (t->type == TOK_DELIMITER && t->length == 1) {
            if (*t->lexeme == '(') found_paren_open++;
            if (*t->lexeme == ')') found_paren_close++;
            if (*t->lexeme == ';') found_semis++;
            if (*t->lexeme == ',') found_comma++;
            if (*t->lexeme == '[') found_brack_open++;
            if (*t->lexeme == ']') found_brack_close++;
        }
        if (t->type == TOK_ARITHMETIC_OP && t->length == 1 && *t->lexeme == '/') found_div++;
        if (t->type == TOK_LOGIC_OP && t->length == 2 && strncmp(t->lexeme, ">=", 2)==0) found_ge++;
        if (t->type == TOK_LOGIC_OP && t->length == 2 && strncmp(t->lexeme, "==", 2)==0) found_eqeq++;
    }

    TEST_ASSERT_TRUE(found_paren_open  >= 1);
    TEST_ASSERT_TRUE(found_paren_close >= 1);
    TEST_ASSERT_TRUE(found_semis       >= 1);
    TEST_ASSERT_TRUE(found_comma       >= 1);
    TEST_ASSERT_TRUE(found_brack_open  >= 1);
    TEST_ASSERT_TRUE(found_brack_close >= 1);

    TEST_ASSERT_TRUE(found_div  >= 1);
    TEST_ASSERT_TRUE(found_ge   >= 1);
    TEST_ASSERT_TRUE(found_eqeq >= 1);

    free_token_list(&list);
}
