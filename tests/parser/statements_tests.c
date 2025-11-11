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

    // 3) Lexer + tokens (IMPORTANTÍSIMO: usar el TokenList del caller)
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
        .interner    = interner,  // si tu struct Parser tiene este campo
        .syms        = syms,      // si tu struct Parser tiene este campo
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

/* ============= Tests de Sentencias ============= */

void test_parser_if_statement(void) {
    const char *source = "If (x > 5) Print(x); End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(1, program->as.program.decls.count);
    
    AstNode *if_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->kind);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.cond);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.then_blk);
    TEST_ASSERT_EQUAL(0, if_stmt->as.if_stmt.elseifs.count);
    TEST_ASSERT_NULL(if_stmt->as.if_stmt.else_blk);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

void test_parser_if_else_statement(void) {
    const char *source = "If (x > 5) Print(x); Else Print(0); End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *if_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->kind);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.cond);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.then_blk);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.else_blk);
    TEST_ASSERT_EQUAL(0, if_stmt->as.if_stmt.elseifs.count);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

void test_parser_if_elseif_else_statement(void) {
    const char *source = "If (x > 10) Print(1); ElseIf (x > 5) Print(2); Else Print(0); End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *if_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_IF, if_stmt->kind);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.cond);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.then_blk);
    TEST_ASSERT_NOT_NULL(if_stmt->as.if_stmt.else_blk);
    TEST_ASSERT_EQUAL(1, if_stmt->as.if_stmt.elseifs.count);
    
    AstNode *elseif = if_stmt->as.if_stmt.elseifs.data[0];
    TEST_ASSERT_EQUAL(AST_IF, elseif->kind);
    TEST_ASSERT_NOT_NULL(elseif->as.if_stmt.cond);
    TEST_ASSERT_NOT_NULL(elseif->as.if_stmt.then_blk);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

void test_parser_while_statement(void) {
    const char *source = "While (x < 10) x += 1; Print(x); End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *while_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_WHILE, while_stmt->kind);
    TEST_ASSERT_NOT_NULL(while_stmt->as.while_stmt.cond);
    TEST_ASSERT_NOT_NULL(while_stmt->as.while_stmt.body);
    
    // El cuerpo debe tener 2 sentencias
    TEST_ASSERT_EQUAL(AST_BLOCK, while_stmt->as.while_stmt.body->kind);
    TEST_ASSERT_EQUAL(2, while_stmt->as.while_stmt.body->as.block.stmts.count);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

void test_parser_for_statement(void) {
    const char *source = "For (Integer i = 0 : i < 10 : i += 1) Print(i); End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *for_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_FOR, for_stmt->kind);
    TEST_ASSERT_NOT_NULL(for_stmt->as.for_stmt.init);
    TEST_ASSERT_NOT_NULL(for_stmt->as.for_stmt.cond);
    TEST_ASSERT_NOT_NULL(for_stmt->as.for_stmt.step);
    TEST_ASSERT_NOT_NULL(for_stmt->as.for_stmt.body);
    
    // La inicialización debe ser una declaración de variable
    TEST_ASSERT_EQUAL(AST_VAR_DECL, for_stmt->as.for_stmt.init->kind);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

void test_parser_print_statement(void) {
    const char *source = "Print(x, y, z);";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *print_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_PRINT, print_stmt->kind);
    TEST_ASSERT_EQUAL(3, print_stmt->as.print.exprs.count);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

void test_parser_read_statement(void) {
    const char *source = "Read(x, y);";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *read_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_READ, read_stmt->kind);
    TEST_ASSERT_EQUAL(2, read_stmt->as.read.targets.count);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

void test_parser_return_statement(void) {
    const char *source = "Return x + y;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *ret_stmt = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_RETURN, ret_stmt->kind);
    TEST_ASSERT_NOT_NULL(ret_stmt->as.ret.expr);
    TEST_ASSERT_EQUAL(AST_BINOP, ret_stmt->as.ret.expr->kind);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

/* ============= Tests de Bloques y Programas ============= */

void test_parser_block_with_multiple_statements(void) {
    const char *source = "Integer x = 5; Print(x); x += 1;";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(AST_PROGRAM, program->kind);
    TEST_ASSERT_EQUAL(3, program->as.program.decls.count);
    
    // Primera sentencia: declaración
    TEST_ASSERT_EQUAL(AST_VAR_DECL, program->as.program.decls.data[0]->kind);
    // Segunda sentencia: print
    TEST_ASSERT_EQUAL(AST_PRINT, program->as.program.decls.data[1]->kind);
    // Tercera sentencia: asignación compuesta
    TEST_ASSERT_EQUAL(AST_EXPR, program->as.program.decls.data[2]->kind);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

void test_parser_nested_blocks(void) {
    const char *source = "If (x > 0) If (y > 0) Print(1); End End";
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    AstNode *outer_if = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_IF, outer_if->kind);
    
    AstNode *outer_block = outer_if->as.if_stmt.then_blk;
    TEST_ASSERT_EQUAL(AST_BLOCK, outer_block->kind);
    TEST_ASSERT_EQUAL(1, outer_block->as.block.stmts.count);
    
    AstNode *inner_if = outer_block->as.block.stmts.data[0];
    TEST_ASSERT_EQUAL(AST_IF, inner_if->kind);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

void test_parser_complete_program(void) {
    const char *source = 
        "Integer factorial(Integer n) "
        "    If (n <= 1) "
        "        Return 1; "
        "    Else "
        "        Return n * factorial(n - 1); "
        "    End "
        "End "
        "Null main() "
        "    Integer result = factorial(5); "
        "    Print(result); "
        "End";
    
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    TEST_ASSERT_EQUAL(AST_PROGRAM, program->kind);
    TEST_ASSERT_EQUAL(2, program->as.program.decls.count);
    
    // Primera función: factorial
    AstNode *factorial = program->as.program.decls.data[0];
    TEST_ASSERT_EQUAL(AST_FUNC_DECL, factorial->kind);
    TEST_ASSERT_EQUAL_STRING("factorial", factorial->as.func_decl.name);
    TEST_ASSERT_EQUAL_STRING("Integer", factorial->as.func_decl.ret_type);
    TEST_ASSERT_EQUAL(1, factorial->as.func_decl.params.count);
    
    // Segunda función: main
    AstNode *main_func = program->as.program.decls.data[1];
    TEST_ASSERT_EQUAL(AST_FUNC_DECL, main_func->kind);
    TEST_ASSERT_EQUAL_STRING("main", main_func->as.func_decl.name);
    TEST_ASSERT_EQUAL_STRING("Null", main_func->as.func_decl.ret_type);
    
    TEST_ASSERT_EQUAL(0, p.error_count);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

/* ============= Tests de Manejo de Errores ============= */

void test_parser_error_handling_missing_semicolon(void) {
    const char *source = "Integer x = 5 Print(x);"; // Falta ; después de la declaración
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    // Debe haber errores
    TEST_ASSERT_TRUE(p.error_count > 0);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

void test_parser_error_handling_missing_end(void) {
    const char *source = "If (x > 5) Print(x);"; // Falta End
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    // Debe haber errores
    TEST_ASSERT_TRUE(p.error_count > 0);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

void test_parser_error_handling_unexpected_token(void) {
    const char *source = "Integer 123 = 5;"; // Nombre de variable inválido
    AstArena *arena;
    Interner *interner;
    Interned *syms;
    TokenList token_list;
    Parser p = create_parser_from_source(source, &arena, &interner, &syms, &token_list);
    
    AstNode *program = parse_program(&p);
    
    TEST_ASSERT_NOT_NULL(program);
    // Debe haber errores
    TEST_ASSERT_TRUE(p.error_count > 0);
    
    cleanup_parser_resources(arena, interner, syms, &token_list);
}

void test_parser_error_recovery(void) {
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