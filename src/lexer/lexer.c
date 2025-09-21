#include "lexer.h"
#include "symbol_table.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

void init_lexer(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->length = strlen(source);
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
}

// Si ya tienes una tabla de keywords en symbol_table.h, puedes exponer una función.
// Aquí dejo una firma opcional; si no la tienes aún, déjala con return 0 y luego la conectas.
static int is_keyword_lexeme(const char *lexeme, size_t len) {
    // TODO: Integra con tu tabla de símbolos/palabras reservadas:
    //   - por ejemplo: return symbol_table_is_keyword_n(lexeme, len);
    (void)lexeme; (void)len;
    return 0;
}

static int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' ||
           c == '\r' || c == '\f' || c == '\v';
}

static void make_error_token_until_ws(Lexer *lexer, Token *token) {
    size_t start_pos = lexer->position;
    size_t i = start_pos;

    // Avanzar hasta espacio en blanco o EOF (sin incluir el blanco)
    while (i < lexer->length && !is_space(lexer->source[i])) {
        i++;
    }

    size_t len = i - start_pos;

    // Completar campos del token
    token->type   = TOK_ERROR;
    token->line   = lexer->line;
    token->column = lexer->column;
    token->length = len;
    token->lexeme = lexer->source + start_pos;

    if (len > 0) {
        lexer->position = i - 1;      // último char del lexema de error
        lexer->column  += (len - 1);  // el bucle sumará 1 más
    }
}


static int is_alpha_uscore(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z');
}

static int is_alnum_uscore(char c) {
    return is_alpha_uscore(c) || (c >= '0' && c <= '9') || c == '_';
}

static void identify_identifier(Lexer *lexer, Token *token) { 
    size_t start_pos = lexer->position;
    size_t i = start_pos;

    if (!is_alpha_uscore(lexer->source[i])) {
        make_error_token_until_ws(lexer, token);
        return;
    }

    i++;
    while (i < lexer->length && is_alnum_uscore(lexer->source[i])) {
        i++;
    }

    size_t len = i - start_pos;

    token->line   = lexer->line;
    token->column = lexer->column;
    token->length = len;
    token->lexeme = lexer->source + start_pos;

    if (is_keyword_lexeme(token->lexeme, token->length)) {
        token->type = TOK_KEYWORD;
    } else {
        token->type = TOK_IDENTIFIER;
    }

    lexer->position = i - 1;
    if (len > 0) {
        lexer->column += (len - 1);
    }
}

static void identify_number(Lexer *lexer, Token *token)    { token->type = TOK_NUMBER; }
static void identify_string(Lexer *lexer, Token *token)    { token->type = TOK_STRING; }
static void identify_delimiter(Lexer *lexer, Token *token) { token->type = TOK_DELIMITER; }
static void identify_operator(Lexer *lexer, Token *token)  { token->type = TOK_LOGIC_OP; }
static void identify_comment(Lexer *lexer, Token *token)   { token->type = TOK_COMMENT; }
static void identify_error(Lexer *lexer, Token *token)     { token->type = TOK_ERROR; }

static void skip_line_comment(Lexer *lexer) {
    lexer->position += 2;
    lexer->column += 2;
    while (lexer->position < lexer->length && lexer->source[lexer->position] != '\n') {
        lexer->position++;
        lexer->column++;
    }
}

static void skip_block_comment(Lexer *lexer) {
    lexer->position += 2;
    lexer->column += 2;
    while (lexer->position < lexer->length) {
        if (lexer->source[lexer->position] == '*' && lexer->source[lexer->position + 1] == '/') {
            lexer->position += 2;
            lexer->column += 2;
            break;
        }
        if (lexer->source[lexer->position] == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        lexer->position++;
    }
}

static void init_token(Token *token) {
    token->type = TOK_ERROR;
    token->lexeme = NULL;
    token->length = 0;
    token->line = 0;
    token->column = 0;
}

void tokenize(Lexer *lexer, TokenList *list) {
    Token token; // Declarar una sola variable token reutilizable

    while (lexer->position < lexer->length) {
        char current = lexer->source[lexer->position];
        init_token(&token); // Inicializar antes de usar

        if (current == '/' && lexer->position + 1 < lexer->length) {
            if (lexer->source[lexer->position + 1] == '/') { skip_line_comment(lexer); continue; }
            if (lexer->source[lexer->position + 1] == '*') { skip_block_comment(lexer); continue; }
        }

        if ((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z')) {
            identify_identifier(lexer, &token);
        } else if (current >= '0' && current <= '9') {
            identify_number(lexer, &token);
        } else if (current == '"') {
            identify_string(lexer, &token);
        } else {
            identify_delimiter(lexer, &token);
        }

        add_token(list, token);
        // Actualización de posición, línea y columna
        lexer->position++;
        if (current == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
    }
}