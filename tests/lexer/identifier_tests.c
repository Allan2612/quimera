#include "unity.h"
#include "lexer.h"
#include "ast.h"
#include "interner.h"
#include "utils.h"
#include "diag_errors.h"
#include <stdio.h>
#include <string.h>

// ===== TESTS DE IDENTIFICADORES =====

void test_tokenize_simple_identifier(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "variable";
    
    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("variable", list.tokens[0].lexeme, strlen("variable"));
    TEST_ASSERT_EQUAL(strlen("variable"), list.tokens[0].length);
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);
    TEST_ASSERT_EQUAL(1, list.tokens[0].column);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_identifier_with_underscore(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "private_var";
    
    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);

    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("private_var", list.tokens[0].lexeme, strlen("private_var"));
    TEST_ASSERT_EQUAL(strlen("private_var"), list.tokens[0].length);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_identifier_with_numbers(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "var123test";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("var123test", list.tokens[0].lexeme, strlen("var123test"));
    TEST_ASSERT_EQUAL(strlen("var123test"), list.tokens[0].length);
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);
    TEST_ASSERT_EQUAL(1, list.tokens[0].column);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_identifier_with_numbers_and_underscores(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "var123_test";
    
    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_IDENTIFIER, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("var123_test", list.tokens[0].lexeme, strlen("var123_test"));
    TEST_ASSERT_EQUAL(strlen("var123_test"), list.tokens[0].length);
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);
    TEST_ASSERT_EQUAL(1, list.tokens[0].column);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}