#include "unity.h"
#include "parser.h"
#include "lexer.h"
#include "utils.h"
#include "ast.h"
#include <string.h>
#include <stdlib.h>

// Helper para crear parser desde código fuente
static Parser create_parser_from_source(const char *source, AstArena **arena_out, Interner **interner_out, Interned **syms_out, TokenList *token_list_out) {
    AstArena *arena = ast_arena_new();
    *arena_out = arena;

    Interner *interner = (Interner*)malloc(sizeof(Interner));
    interner_init(interner, arena, 256);
    *interner_out = interner;

    Interned *syms = (Interned*)malloc(sizeof(Interned));
    interner_bootstrap_quimera(interner, syms);
    *syms_out = syms;

    Lexer lexer;
    init_lexer(&lexer, source, interner, syms);

    init_token_list(token_list_out);
    tokenize(&lexer, token_list_out);

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

/* ============= Tests de Sentencias IF ============= */

// Test: IF simple
void test_parser_if_simple(void) {
    const char *source = "If (x > 5) Print(x); End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *if_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->kind);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.cond);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.then_blk);
    TEST_ASSERT_EQUAL(0, if_stmt->as.if_stmt.elseifs.count);
    TEST_ASSERT_NULL(if_stmt->as.if_stmt.else_blk);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: IF-ELSE
void test_parser_if_else(void) {
    const char *source = "If (x > 5) Print(\"greater\"); Else Print(\"not greater\"); End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *if_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->kind);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.cond);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.then_blk);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.else_blk);
    TEST_ASSERT_EQUAL(0, if_stmt->as.if_stmt.elseifs.count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: IF-ELSEIF-ELSE
void test_parser_if_elseif_else(void) {
    const char *source = "If (x > 10) Print(\"big\"); ElseIf (x > 5) Print(\"medium\"); Else Print(\"small\"); End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *if_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->kind);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.cond);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.then_blk);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.else_blk);
    TEST_ASSERT_EQUAL(1, if_stmt->as.if_stmt.elseifs.count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: IF anidados
void test_parser_if_nested(void) {
    const char *source = "If (x > 0) If (y > 0) Print(\"both positive\"); End End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

/* ============= Tests de Sentencias WHILE ============= */

// Test: WHILE simple
void test_parser_while_simple(void) {
    const char *source = "While (x < 10) x = x + 1; End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *while_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_WHILE, while_stmt->kind);
    TEST_ASSERT_NOT_NULL(while_stmt->as.while_stmt.cond);
    TEST_ASSERT_NOT_NULL(while_stmt->as.while_stmt.body);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: WHILE anidado
void test_parser_while_nested(void) {
    const char *source = "While (i < 5) While (j < 3) j = j + 1; End i = i + 1; End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

/* ============= Tests de Sentencias FOR ============= */

// Test: FOR con declaración
void test_parser_for_with_declaration(void) {
    const char *source = "For (Integer i = 0 : i < 10 : i = i + 1) Print(i); End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *for_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_FOR, for_stmt->kind);
    TEST_ASSERT_NOT_NULL(for_stmt->as.for_stmt.init);
    TEST_ASSERT_NOT_NULL(for_stmt->as.for_stmt.cond);
    TEST_ASSERT_NOT_NULL(for_stmt->as.for_stmt.step);
    TEST_ASSERT_NOT_NULL(for_stmt->as.for_stmt.body);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: FOR con asignación
void test_parser_for_with_assignment(void) {
    const char *source = "For (i = 0 : i < 10 : i = i + 1) Print(i); End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

/* ============= Tests de Sentencias PRINT y READ ============= */

// Test: PRINT simple
void test_parser_print_simple(void) {
    const char *source = "Print(\"Hello, World!\");";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *print_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_PRINT, print_stmt->kind);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: PRINT con variable
void test_parser_print_variable(void) {
    const char *source = "Print(x);";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: READ
void test_parser_read(void) {
    const char *source = "Read(x);";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *read_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_READ, read_stmt->kind);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

/* ============= Tests de Sentencias RETURN ============= */

// Test: RETURN con expresión
void test_parser_return_with_expression(void) {
    const char *source = "Return x + 1;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *return_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->kind);
    TEST_ASSERT_NOT_NULL(return_stmt->as.ret.expr);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

// Test: RETURN sin expresión
void test_parser_return_without_expression(void) {
    const char *source = "Return;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *return_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_RETURN, return_stmt->kind);
    // El valor puede ser NULL para return vacío
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

/* ============= Tests de Combinaciones Complejas ============= */

// Test: Programa complejo con múltiples estructuras
void test_parser_complex_program(void) {
    const char *source = 
        "Integer factorial(Integer n) "
        "If (n <= 1) Return 1; "
        "Else Return n * factorial(n - 1); "
        "End End "
        "Integer main() "
        "Integer x = 5; "
        "Integer result = factorial(x); "
        "Print(result); "
        "Return 0; "
        "End";
    
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(0, p.error_count);
    TEST_ASSERT_EQUAL(2, program->as.program.decls.count); // factorial y main
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}