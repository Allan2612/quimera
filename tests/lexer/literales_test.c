#include "unity.h"
#include "lexer.h"
#include "ast.h"
#include "interner.h"
#include "utils.h"
#include "diag_errors.h"
#include <stdio.h>
#include <string.h>

// ===== TESTS DE LITERALES (STRINGS Y CHARS) =====

void test_tokenize_simple_string(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "\"hello world\"";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("\"hello world\"", list.tokens[0].lexeme, 13);
    TEST_ASSERT_EQUAL(13, list.tokens[0].length);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_empty_string_literal(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "\"\"";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("\"\"", list.tokens[0].lexeme, 2);
    TEST_ASSERT_EQUAL(2, list.tokens[0].length);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_simple_char(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "'a'";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_CHAR, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("'a'", list.tokens[0].lexeme, 3);
    TEST_ASSERT_EQUAL(3, list.tokens[0].length);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_empty_char(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "''";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ERROR, list.tokens[0].type);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_char_escape_sequences(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "'\\n'";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_CHAR, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("'\\n'", list.tokens[0].lexeme, 4);
    TEST_ASSERT_EQUAL(4, list.tokens[0].length);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_char_escape_tab(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "'\\t'";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_CHAR, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("'\\t'", list.tokens[0].lexeme, 4);
    TEST_ASSERT_EQUAL(4, list.tokens[0].length);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_char_escape_backslash(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "'\\\\'";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_CHAR, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("'\\\\'", list.tokens[0].lexeme, 4);
    TEST_ASSERT_EQUAL(4, list.tokens[0].length);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_string_with_escapes(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "\"Hello\\nWorld\"";
    
    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[0].type);
    TEST_ASSERT_EQUAL_STRING_LEN("\"Hello\\nWorld\"", list.tokens[0].lexeme, 14);
    TEST_ASSERT_EQUAL(14, list.tokens[0].length);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

// ===== TESTS DE ERRORES EN LITERALES =====

void test_tokenize_unclosed_string(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "\"unclosed string";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ERROR, list.tokens[0].type);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_unclosed_char(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "'unclosed";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ERROR, list.tokens[0].type);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_multichar_invalid(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "'abc'";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ERROR, list.tokens[0].type);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_tokenize_invalid_escape_sequence(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;
    const char *source = "'\\x'";

    init_lexer(&lexer, source, &I, &S);
    init_token_list(&list);
    
    tokenize(&lexer, &list);
    
    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_ERROR, list.tokens[0].type);
    
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}

void test_literal_position_with_escapes_and_real_newlines(void) {
    AstArena *arena = ast_arena_new();

    Interner I;
    interner_init(&I, arena, 256);

    Interned S;
    interner_bootstrap_quimera(&I, &S);

    Lexer lexer;
    TokenList list;

    // Caso 1: el literal contiene una secuencia de escape "\n" -> NO avanza línea
    const char *source1 = "\"Hola\\nMundo\"";
    init_lexer(&lexer, source1, &I, &S);
    init_token_list(&list);
    tokenize(&lexer, &list);

    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[0].type);
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);   // sigue en la misma línea
    TEST_ASSERT_EQUAL(1, list.tokens[0].column); // inicia en col 1
    free_token_list(&list);

    // Caso 2: el literal incluye un salto de línea real dentro del texto fuente
    const char *source2 = "\"Hola\nMundo\"";
    init_lexer(&lexer, source2, &I, &S);
    init_token_list(&list);
    tokenize(&lexer, &list);

    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[0].type);
    // Debe haber incrementado la línea dentro del literal
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);     // empieza en línea 1
    TEST_ASSERT_EQUAL(1, list.tokens[0].column);   // columna inicial sigue siendo 1
    TEST_ASSERT_TRUE(lexer.line > 1);              // el lexer ahora en línea 2
    free_token_list(&list);

    // Caso 3 (combinado): literal con escape y luego salto real
    const char *source3 = "\"Linea1\\n\nLinea2\"";
    init_lexer(&lexer, source3, &I, &S);
    init_token_list(&list);
    tokenize(&lexer, &list);

    TEST_ASSERT_EQUAL(1, list.count);
    TEST_ASSERT_EQUAL(TOK_STRING, list.tokens[0].type);
    TEST_ASSERT_EQUAL(1, list.tokens[0].line);
    TEST_ASSERT_TRUE(lexer.line > 1); // debe haberse avanzado de línea real
    interner_destroy(&I);
    free_token_list(&list);
    ast_arena_free(arena);
}