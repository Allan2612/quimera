#include "unity.h"
#include "lexer.h"
#include "ast.h"
#include "interner.h"
#include "utils.h"
#include "diag.h"
#include <stdio.h>
#include <string.h>

// ===== TESTS DE PALABRAS RESERVADAS =====

void test_tokenize_keywords_if(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "If ElseIf Else End";
    
    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(4, list.count);

    const char *expected_lexemes[] = { "If", "ElseIf", "Else", "End" };
    for (size_t i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL(TOK_KEYWORD, list.tokens[i].type);
        TEST_ASSERT_EQUAL_STRING_LEN(expected_lexemes[i], list.tokens[i].lexeme, strlen(expected_lexemes[i]));
        TEST_ASSERT_EQUAL(strlen(expected_lexemes[i]), list.tokens[i].length);
        TEST_ASSERT_EQUAL(1, list.tokens[i].line);
        size_t expected_column = 1;
        for (size_t j = 0; j < i; j++) {
            expected_column += strlen(expected_lexemes[j]) + 1; // +1 por el espacio
        }
        TEST_ASSERT_EQUAL(expected_column, list.tokens[i].column);
    }
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_keywords_types(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "Integer Float Boolean Char String Null";
    
    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);

    TEST_ASSERT_EQUAL(6, list.count);
    const char *expected_lexemes[] = { "Integer", "Float", "Boolean", "Char", "String", "Null" };
    for (size_t i = 0; i < 6; i++) {
        TEST_ASSERT_EQUAL(TOK_KEYWORD, list.tokens[i].type);
        TEST_ASSERT_EQUAL_STRING_LEN(expected_lexemes[i], list.tokens[i].lexeme, strlen(expected_lexemes[i]));
        TEST_ASSERT_EQUAL(strlen(expected_lexemes[i]), list.tokens[i].length);
        TEST_ASSERT_EQUAL(1, list.tokens[i].line);
        size_t expected_column = 1;
        for (size_t j = 0; j < i; j++) {
            expected_column += strlen(expected_lexemes[j]) + 1; // +1 por el espacio
        }
        TEST_ASSERT_EQUAL(expected_column, list.tokens[i].column);
    }

    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_keywords_loop(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "For While End";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(3, list.count);
    const char *expected_lexemes[] = { "For", "While", "End" };
    for (size_t i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL(TOK_KEYWORD, list.tokens[i].type);
        TEST_ASSERT_EQUAL_STRING_LEN(expected_lexemes[i], list.tokens[i].lexeme, strlen(expected_lexemes[i]));
        TEST_ASSERT_EQUAL(strlen(expected_lexemes[i]), list.tokens[i].length);
        TEST_ASSERT_EQUAL(1, list.tokens[i].line);
        size_t expected_column = 1;
        for (size_t j = 0; j < i; j++) {
            expected_column += strlen(expected_lexemes[j]) + 1; // +1 por el espacio
        }
        TEST_ASSERT_EQUAL(expected_column, list.tokens[i].column);
    }

    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_keywords_functions(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "Return";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    const char *expected_lexemes[] = { "Return" };
    for (size_t i = 0; i < 1; i++) {
        TEST_ASSERT_EQUAL(TOK_KEYWORD, list.tokens[i].type);
        TEST_ASSERT_EQUAL_STRING_LEN(expected_lexemes[i], list.tokens[i].lexeme, strlen(expected_lexemes[i]));
        TEST_ASSERT_EQUAL(strlen(expected_lexemes[i]), list.tokens[i].length);
        TEST_ASSERT_EQUAL(1, list.tokens[i].line);
        size_t expected_column = 1;
        for (size_t j = 0; j < i; j++) {
            expected_column += strlen(expected_lexemes[j]) + 1; // +1 por el espacio
        }
        TEST_ASSERT_EQUAL(expected_column, list.tokens[i].column);
    }

    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_keywords_io(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "Print Read";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(2, list.count);
    const char *expected_lexemes[] = { "Print", "Read" };
    for (size_t i = 0; i < 2; i++) {
        TEST_ASSERT_EQUAL(TOK_KEYWORD, list.tokens[i].type);
        TEST_ASSERT_EQUAL_STRING_LEN(expected_lexemes[i], list.tokens[i].lexeme, strlen(expected_lexemes[i]));
        TEST_ASSERT_EQUAL(strlen(expected_lexemes[i]), list.tokens[i].length);
        TEST_ASSERT_EQUAL(1, list.tokens[i].line);
        size_t expected_column = 1;
        for (size_t j = 0; j < i; j++) {
            expected_column += strlen(expected_lexemes[j]) + 1; // +1 por el espacio
        }
        TEST_ASSERT_EQUAL(expected_column, list.tokens[i].column);
    }

    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_keywords_boolean_literals(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "True False";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(2, list.count);
    const char *expected_lexemes[] = { "True", "False" };
    for (size_t i = 0; i < 2; i++) {
        TEST_ASSERT_EQUAL(TOK_KEYWORD, list.tokens[i].type);
        TEST_ASSERT_EQUAL_STRING_LEN(expected_lexemes[i], list.tokens[i].lexeme, strlen(expected_lexemes[i]));
        TEST_ASSERT_EQUAL(strlen(expected_lexemes[i]), list.tokens[i].length);
        TEST_ASSERT_EQUAL(1, list.tokens[i].line);
        size_t expected_column = 1;
        for (size_t j = 0; j < i; j++) {
            expected_column += strlen(expected_lexemes[j]) + 1; // +1 por el espacio
        }
        TEST_ASSERT_EQUAL(expected_column, list.tokens[i].column);
    }

    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}