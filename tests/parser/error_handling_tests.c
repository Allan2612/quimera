#include "unity.h"
#include "parser.h"
#include "lexer.h"
#include "utils.h"
#include "ast.h"
#include <string.h>
#include <stdlib.h>

// Helper para crear parser desde código fuente
static Parser create_parser_from_source(const char *source, AstArena **arena_out, Interner **interner_out, Interned **syms_out, TokenList *token_list_out) {
    // 1) Arena
    AstArena *arena = ast_arena_new();
    *arena_out = arena;

    // 2) Interner + símbolos internados
    Interner *interner = (Interner*)malloc(sizeof(Interner));
    interner_init(interner, arena, 256);
    *interner_out = interner;

    Interned *syms = (Interned*)malloc(sizeof(Interned));
    interner_bootstrap_quimera(interner, syms);
    *syms_out = syms;

    // 3) Lexer + tokens
    Lexer lexer;
    init_lexer(&lexer, source, interner, syms);

    init_token_list(token_list_out);
    tokenize(&lexer, token_list_out);

    // 4) Parser
    Parser parser = {
        .toks        = token_list_out->tokens,
        .count       = token_list_out->count,
        .i           = 0,
        .arena       = arena,
        .interner    = interner,
        .syms        = syms,
        .error_count = 0
    };
    return parser;
}

// Helper para limpiar recursos
static void cleanup_parser_resources(AstArena *arena, Interner *interner, Interned *syms, TokenList *tokens) {
    if (arena) ast_arena_free(arena);
    if (interner) {
        interner_destroy(interner);
        free(interner);
    }
    if (syms) free(syms);
    if (tokens) free_token_list(tokens);
}

/* ============= Tests de Manejo de Errores ============= */

// Test: IF sin End
void test_parser_if_missing_end(void) {
    const char *source = "If (x > 5) Print(x);"; // Falta End
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_TRUE(p.error_count > 0); // Debe haber al menos un error
    
    // El programa debe seguir siendo parseable aunque con errores
    TEST_ASSERT_TRUE(program->as.program.decls.count >= 1);
    AstNode *if_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->kind);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: WHILE sin End
void test_parser_while_missing_end(void) {
    const char *source = "While (x < 10) x = x + 1;"; // Falta End
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_TRUE(p.error_count > 0);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: FOR sin End
void test_parser_for_missing_end(void) {
    const char *source = "For (Integer i = 0 : i < 10 : i = i + 1) Print(i);"; // Falta End
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_TRUE(p.error_count > 0);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: IF con paréntesis faltante
void test_parser_if_missing_parenthesis(void) {
    const char *source = "If x > 5 Print(x); End"; // Faltan paréntesis
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_TRUE(p.error_count > 0);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: Declaración con tipo faltante
void test_parser_declaration_missing_type(void) {
    const char *source = "x = 10;"; // Falta tipo
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    // Esto debe parsearse como una asignación, no como declaración
    // El parser debe tratar de continuar
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: Token inesperado
void test_parser_unexpected_token(void) {
    const char *source = "Integer 123 = 5;"; // 123 no es un identificador válido
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_TRUE(p.error_count > 0);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: Recuperación de errores - múltiples errores en secuencia
void test_parser_error_recovery_multiple(void) {
    const char *source = "Integer x = 5; Integer y =; Integer z = 10;"; // Error en y
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    // Debe haber errores pero el parser debe continuar
    TEST_ASSERT_TRUE(p.error_count > 0);
    // Debe haber al menos algunas declaraciones parseadas
    TEST_ASSERT_TRUE(program->as.program.decls.count >= 1);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: EOF inesperado
void test_parser_unexpected_eof(void) {
    const char *source = "Integer x ="; // EOF inesperado
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_TRUE(p.error_count > 0);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: Bloques anidados con errores
void test_parser_nested_blocks_with_errors(void) {
    const char *source = "If (x > 0) If (y > 0) Print(x); Print(y); End"; // Falta un End
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_TRUE(p.error_count > 0);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}