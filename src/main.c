#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "diag_errors.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Inicializar sistema de diagnósticos
    diag_init(false, stderr);

    // Crear arena para AST
    AstArena *arena = ast_arena_new();
    if (!arena) {
        fprintf(stderr, "Error: No se pudo crear la arena AST\n");
        return 1;
    }

    // Crear interner
    Interner interner;
    interner_init(&interner, arena, 64);
    
    Interned symbols;
    interner_bootstrap_quimera(&interner, &symbols);

    // Código de ejemplo en Quimera
    const char *source = 
        "Integer x = 10;\n"
        "Integer y = 5;\n"
        "Integer resultado = x + y;\n"
        "Boolean esMayor = x > y;\n"
        "Print(\"x =\", x);\n"
        "Print(\"y =\", y);\n"
        "Print(\"resultado =\", resultado);\n"
        "Print(\"x > y es\", esMayor);\n"
        "\n"
        "Float pi = 3.14;\n"
        "Float area = pi * 2.0;\n"
        "Print(\"Área =\", area);\n"
        "\n"
        "Boolean verdad = True;\n"
        "Boolean falso = False;\n"
        "Boolean combinado = verdad And falso;\n"
        "Print(\"True And False =\", combinado);\n";

    printf("Código fuente:\n%s\n", source);
    printf("Interpretando...\n");

    // Inicializar lexer
    Lexer lexer;
    init_lexer(&lexer, source, &interner, &symbols);

    // Tokenizar
    TokenList tokens;
    init_token_list(&tokens);
    tokenize(&lexer, &tokens);

    if (tokens.count == 0) {
        printf("Error: No se generaron tokens\n");
        goto cleanup;
    }

    // Parsear
    Parser parser = {
        .toks = tokens.tokens,
        .count = tokens.count,
        .i = 0,
        .arena = arena,
        .interner = &interner,
        .syms = &symbols,
        .error_count = 0
    };

    AstNode *program = parse_program(&parser);
    
    if (!program || parser.error_count > 0) {
        printf("Error: Falló el parsing\n");
        goto cleanup;
    }

    printf("AST parseado exitosamente\n");

    // Interpretar
    InterpreterContext ctx;
    interpreter_init(&ctx, arena, &interner, &symbols);

    EvalResult result = interpret_program(&ctx, program);

    if (result.flow == FLOW_ERROR || ctx.has_error) {
        printf("Error durante la interpretación\n");
    } else {
        printf("Programa ejecutado exitosamente\n");
    }

    interpreter_destroy(&ctx);

cleanup:
    free_token_list(&tokens);
    interner_destroy(&interner);
    ast_arena_free(arena);
    diag_shutdown();

    return 0;
}