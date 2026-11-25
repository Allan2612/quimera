#include "unity.h"
#include "lexer.h"
#include "ast.h"
#include "interner.h"
#include "utils.h"
#include "diag.h"
#include <stdio.h>
#include <string.h>

// ===== TESTS DE FUNCIONES HELPER =====

void test_lexer_init(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    const char *source = "test input";

    init_lexer(&lexer, source, &I, &S);

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
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "If variable 123";

    init_lexer(&lexer, source, &I, &S);
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
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_identifier_and_number_mixed(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "count 42 name";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(3, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL(TOK_INTEGER, list.tokens[1].type);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[2].type);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

// ===== TESTS DE MANEJO DE ESPACIOS =====

void test_tokenize_with_spaces(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "  variable   123  ";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(2, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL(TOK_INTEGER, list.tokens[1].type);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_with_tabs_and_newlines(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "var1\t\nvar2";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(2, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("var1", list.tokens[0].lexeme, 4);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[1].type);
    TEST_ASSERT_EQUAL_STRING_LEN("var2", list.tokens[1].lexeme, 4);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

// ===== TESTS DE CASOS EDGE =====

void test_tokenize_empty_string(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(0, list.count);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_only_spaces(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "   \t\n  ";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(0, list.count);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_single_character_identifier(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "a";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("a", list.tokens[0].lexeme, 1);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

// ===== TESTS DE POSICIÓN Y LÍNEAS =====

void test_lexer_position_tracking(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "var1\nvar2";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(2, list.count);
    
    // Primer token en línea 1
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);
    TEST_ASSERT_EQUAL(1, list.tokens[0].column);
    
    // Segundo token en línea 2
    TEST_ASSERT_EQUAL(2, list.tokens[1].line);
    TEST_ASSERT_EQUAL(1, list.tokens[1].column);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_lexer_position_tracking_all_token_types(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;

    const char *source =
        "if\n"                // TOK_KEYWORD (línea 1, col 1)
        "var\n"               // TOK_IDENTIFIER (línea 2, col 1)
        "123\n"               // TOK_INTEGER (línea 3, col 1)
        "45.6\n"              // TOK_FLOAT (línea 4, col 1)
        "\"str\"\n"           // TOK_STRING (línea 5, col 1)
        "'a'\n"               // TOK_CHAR (línea 6, col 1)
        "(\n"                 // TOK_DELIMITER (línea 7, col 1)
        ")\n"                 // TOK_DELIMITER (línea 8, col 1)
        "{\n"                 // TOK_DELIMITER (línea 9, col 1)
        "}\n"                 // TOK_DELIMITER (línea 10, col 1)
        "+\n"                 // TOK_ARITHMETIC_OP (línea 11, col 1)
        "-\n"                 // TOK_ARITHMETIC_OP (línea 12, col 1)
        "*\n"                 // TOK_ARITHMETIC_OP (línea 13, col 1)
        "/\n"                 // TOK_ARITHMETIC_OP (línea 14, col 1)
        "==\n"                // TOK_LOGIC_OP (línea 15, col 1)
        ">=\n"                // TOK_LOGIC_OP (línea 16, col 1)
        "=\n"                 // TOK_ASSIGNMENT_OP (línea 17, col 1)
        ";\n";                // TOK_DELIMITER (línea 18, col 1)

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);

    tokenize(&lexer, &list);

    // Verifica tipo, línea y columna de cada token
    TEST_ASSERT_EQUAL(18, list.count);

    TEST_ASSERT_EQUAL(TOK_KEYWORD,        list.tokens[0].type);  TEST_ASSERT_EQUAL(1,  list.tokens[0].line);  TEST_ASSERT_EQUAL(1, list.tokens[0].column);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER,     list.tokens[1].type);  TEST_ASSERT_EQUAL(2,  list.tokens[1].line);  TEST_ASSERT_EQUAL(1, list.tokens[1].column);
    TEST_ASSERT_EQUAL(TOK_INTEGER,        list.tokens[2].type);  TEST_ASSERT_EQUAL(3,  list.tokens[2].line);  TEST_ASSERT_EQUAL(1, list.tokens[2].column);
    TEST_ASSERT_EQUAL(TOK_FLOAT,          list.tokens[3].type);  TEST_ASSERT_EQUAL(4,  list.tokens[3].line);  TEST_ASSERT_EQUAL(1, list.tokens[3].column);
    TEST_ASSERT_EQUAL(TOK_STRING,         list.tokens[4].type);  TEST_ASSERT_EQUAL(5,  list.tokens[4].line);  TEST_ASSERT_EQUAL(1, list.tokens[4].column);
    TEST_ASSERT_EQUAL(TOK_CHAR,           list.tokens[5].type);  TEST_ASSERT_EQUAL(6,  list.tokens[5].line);  TEST_ASSERT_EQUAL(1, list.tokens[5].column);
    TEST_ASSERT_EQUAL(TOK_DELIMITER,      list.tokens[6].type);  TEST_ASSERT_EQUAL(7,  list.tokens[6].line);  TEST_ASSERT_EQUAL(1, list.tokens[6].column);
    TEST_ASSERT_EQUAL(TOK_DELIMITER,      list.tokens[7].type);  TEST_ASSERT_EQUAL(8,  list.tokens[7].line);  TEST_ASSERT_EQUAL(1, list.tokens[7].column);
    TEST_ASSERT_EQUAL(TOK_DELIMITER,      list.tokens[8].type);  TEST_ASSERT_EQUAL(9,  list.tokens[8].line);  TEST_ASSERT_EQUAL(1, list.tokens[8].column);
    TEST_ASSERT_EQUAL(TOK_DELIMITER,      list.tokens[9].type);  TEST_ASSERT_EQUAL(10, list.tokens[9].line);  TEST_ASSERT_EQUAL(1, list.tokens[9].column);
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP,  list.tokens[10].type); TEST_ASSERT_EQUAL(11, list.tokens[10].line); TEST_ASSERT_EQUAL(1, list.tokens[10].column);
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP,  list.tokens[11].type); TEST_ASSERT_EQUAL(12, list.tokens[11].line); TEST_ASSERT_EQUAL(1, list.tokens[11].column);
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP,  list.tokens[12].type); TEST_ASSERT_EQUAL(13, list.tokens[12].line); TEST_ASSERT_EQUAL(1, list.tokens[12].column);
    TEST_ASSERT_EQUAL(TOK_ARITHMETIC_OP,  list.tokens[13].type); TEST_ASSERT_EQUAL(14, list.tokens[13].line); TEST_ASSERT_EQUAL(1, list.tokens[13].column);
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP,       list.tokens[14].type); TEST_ASSERT_EQUAL(15, list.tokens[14].line); TEST_ASSERT_EQUAL(1, list.tokens[14].column);
    TEST_ASSERT_EQUAL(TOK_LOGIC_OP,       list.tokens[15].type); TEST_ASSERT_EQUAL(16, list.tokens[15].line); TEST_ASSERT_EQUAL(1, list.tokens[15].column);
    TEST_ASSERT_EQUAL(TOK_ASSIGNMENT_OP,  list.tokens[16].type); TEST_ASSERT_EQUAL(17, list.tokens[16].line); TEST_ASSERT_EQUAL(1, list.tokens[16].column);
    TEST_ASSERT_EQUAL(TOK_DELIMITER,      list.tokens[17].type); TEST_ASSERT_EQUAL(18, list.tokens[17].line); TEST_ASSERT_EQUAL(1, list.tokens[17].column);

    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

// ===== TESTS MIXTOS CON LITERALES =====

void test_tokenize_mixed_with_literals(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "variable \"string\" 'c' 123";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(4, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[1].type);
    TEST_ASSERT_EQUAL(TOK_CHAR, list.tokens[2].type);
    TEST_ASSERT_EQUAL(TOK_INTEGER, list.tokens[3].type);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

// ===== TESTS MIXTOS CON DELIMITADORES =====

void test_mixed_identifiers_numbers_and_delimiters(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "Func(x:Integer, y:Float){Print(x);Print(y);}";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    tokenize(&lexer, &list);

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
    TEST_ASSERT_TRUE(found_print_kw >= 2); 

    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_delimiters_not_confused_with_operators(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;

    const char *source = "a/(b+c); d>=e; f==g, h[i]";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    tokenize(&lexer, &list);

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

    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}
