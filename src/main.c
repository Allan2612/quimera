#include "lexer.h"
#include "parser.h"
#include "analyzer.h"
#include "interpreter.h"
#include "diag.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // Necesario para malloc y free

// Cargar el código desde el archivo en lugar del hardcodeado
static char *read_entire_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long n = ftell(f);
    if (n < 0) { fclose(f); return NULL; }
    rewind(f);
    char *buf = (char*)malloc((size_t)n + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t rd = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[rd] = '\0';
    if (out_len) *out_len = rd;
    return buf;
}

int main(int argc, char *argv[]) {
    // Inicializar el sistema de diagnóstico (sin buffering)
    diag_init();

    // Verificar argumentos
    if (argc < 2) {
        diag_error_simple(STAGE_SYSTEM, 5001, "Uso: quimera [archivo.qmr] | --interactive");
        return 1;
    }

    const char *source_path = NULL;
    bool interactive_mode = false;

    // Parsear argumentos
    if (argc == 2 && strcmp(argv[1], "--interactive") == 0) {
        interactive_mode = true;
    } else if (argc >= 2) {
        source_path = argv[1];
    }

    // Si no es modo interactivo, debe tener archivo
    if (!interactive_mode && !source_path) {
        diag_error_simple(STAGE_SYSTEM, 5001, "Debe especificar un archivo o usar --interactive");
        return 1;
    }

    // Crear arena para AST
    AstArena *arena = ast_arena_new();
    if (!arena) {
        diag_error_memory();
        return 1;
    }

    // Crear interner
    Interner interner;
    interner_init(&interner, arena, 64);
    
    Interned symbols;
    interner_bootstrap_quimera(&interner, &symbols);

    char *source = NULL;
    size_t len = 0;

    if (interactive_mode) {
        // Modo interactivo - leer desde archivo temporal del IDE
        // El IDE creará tmp/ide_input.qmr con el código a ejecutar
        source_path = "tmp/ide_input.qmr";
        source = read_entire_file(source_path, &len);
        
        if (!source) {
            diag_error_file(source_path, "leer archivo temporal del IDE");
            goto cleanup;
        }
    } else {
        // Modo normal - leer archivo especificado
        source = read_entire_file(source_path, &len);
        if (!source) {
            diag_error_file(source_path, "abrir");
            goto cleanup;
        }
    }

    // Inicializar lexer
    Lexer lexer;
    init_lexer(&lexer, source, &interner, &symbols);

    // Tokenizar
    TokenList tokens;
    init_token_list(&tokens);
    tokenize(&lexer, &tokens);

    if (tokens.count == 0) {
        output_program_error("No se generaron tokens");
        free(source);
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
        output_program_error("Error durante el análisis sintáctico");
        free(source);
        goto cleanup;
    }

    // Análisis semántico
    SemanticAnalyzer analyzer;
    analyzer_init(&analyzer, arena, &interner, &symbols);
    
    bool semantic_ok = analyze_program(&analyzer, program);
    
    if (!semantic_ok || analyzer.error_count > 0) {
        output_program_error("Error durante el análisis semántico");
        analyzer_destroy(&analyzer);
        free(source);
        goto cleanup;
    }

    // Interpretar
    InterpreterContext ctx;
    interpreter_init(&ctx, arena, &interner, &symbols);

    EvalResult result = interpret_program(&ctx, program);

    if (result.flow == FLOW_ERROR || ctx.has_error) {
        output_program_error("Error durante la ejecución del programa");
    }

    // Limpiar analizador
    analyzer_destroy(&analyzer);
    interpreter_destroy(&ctx);

cleanup:
    if (source) free(source);
    free_token_list(&tokens);
    interner_destroy(&interner);
    ast_arena_free(arena);
    diag_shutdown();

    return 0;
}