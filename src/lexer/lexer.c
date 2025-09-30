#include "lexer.h"
#include "symbol_table.h"
#include "utils.h"
#include "diag_errors.h"
#include <stdlib.h>
#include <string.h>

//------------Helper Functions----------------

static int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' ||
           c == '\r' || c == '\f' || c == '\v';
}


static int is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z');
}

static int is_alnum_uscore(char c) {
    return is_alpha(c) || ((c >= '0' && c <= '9') || c == '_');
}

static int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

//------------Private Lexer Functions----------------

// Función para verificar si un lexeme es una palabra reservada
static int is_keyword_lexeme(const char *lexeme, size_t len) {
    for (size_t i = 0; i < ReservedWordCount; i++) {
        const char *keyword = ReservedWords[i];
        size_t keyword_len = strlen(keyword);
        
        // Comparar longitud y contenido
        if (len == keyword_len && strncmp(lexeme, keyword, len) == 0) {
            return 1;
        }
    }
    return 0;
}

// Función para identificar lexemas de identificadores o palabras reservadas
static void identify_identifier(Lexer *lexer, Token *token) { 
    size_t start_pos = lexer->position;
    size_t i = start_pos;

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

static void identify_number(Lexer *lexer, Token *token) { 
    size_t start_pos = lexer->position;
    size_t i = start_pos;
    int has_decimal = 0;

    i++;
    while (i < lexer->length && (is_digit(lexer->source[i]) || lexer->source[i] == '.')) {
        if (lexer->source[i] == '.') {
            has_decimal ++;
        }
        i++;
    }

    size_t len = i - start_pos;

    token->line   = lexer->line;
    token->column = lexer->column;
    token->length = len;
    token->lexeme = lexer->source + start_pos;

    if (has_decimal == 1) {
        token->type = TOK_FLOAT;
    } else if (has_decimal > 1) {
        token->type = TOK_ERROR; // Número con más de un punto decimal
        ErrorToken t = { .code=1004, .lexeme=lexer->source+start_pos, .length=len, .line=lexer->line, .column=lexer->column };
        diag_report(ST_LEXER, SV_ERROR, t, NULL);
    } else {
        token->type = TOK_INTEGER;
    }

    lexer->position = i - 1;
    if (len > 0) {
        lexer->column += (len - 1);
    }
}

static void identify_literal(Lexer *lexer, Token *token) {
    size_t start_pos = lexer->position;
    size_t i = start_pos;
    bool is_char = (lexer->source[start_pos] == '\'');
    char quote_type = lexer->source[start_pos];

    i++; // Saltar comilla inicial
    
    // Buscar comilla de cierre
    while (i < lexer->length && lexer->source[i] != quote_type) {
        // Manejar secuencias de escape básicas
        if (lexer->source[i] == '\\' && i + 1 < lexer->length) {
            i += 2; // Saltar el backslash y el siguiente carácter
        } else {
            i++;
        }
    }
    
    // Verificar si encontramos la comilla de cierre
    if (i >= lexer->length) {
        token->type = TOK_ERROR;
        token->line = lexer->line;
        token->column = lexer->column;
        token->length = i - start_pos;
        token->lexeme = lexer->source + start_pos;
        ErrorToken t = { .code=1006, .lexeme=lexer->source+start_pos, .length=i - start_pos, .line=lexer->line, .column=lexer->column };
        diag_report(ST_LEXER, SV_ERROR, t, NULL);
        return;
    }
 
    size_t len = i - start_pos + 1; // Incluir comilla de cierre
    size_t content_len = i - start_pos - 1; // Longitud del contenido sin comillas

    token->line = lexer->line;
    token->column = lexer->column;
    token->length = content_len;
    token->lexeme = lexer->source + start_pos + 1; // Excluir comilla inicial

    if (is_char) {
        // Validaciones más específicas para caracteres
        if (content_len == 0) {
            // Carácter vacío: ''
            token->type = TOK_ERROR;
            ErrorToken t = { .code=1007, .lexeme=lexer->source+start_pos, .length=len, .line=lexer->line, .column=lexer->column };
            diag_report(ST_LEXER, SV_ERROR, t, NULL);
            return;
        } else if (content_len == 1) {
            // Carácter normal: 'a'
            token->type = TOK_CHAR;
        } else if (content_len == 2 && lexer->source[start_pos + 1] == '\\') {
            // Secuencia de escape: '\n', '\t', etc.
            char escape_char = lexer->source[start_pos + 2];
            if (escape_char == 'n' || escape_char == 't' || escape_char == 'r' || 
                escape_char == '\\' || escape_char == '\'' || escape_char == '"' ||
                escape_char == '0') {
                token->type = TOK_CHAR;
            } else {
                // Secuencia de escape inválida
                token->type = TOK_ERROR;
                ErrorToken t = { .code=1008, .lexeme=lexer->source+start_pos, .length=len, .line=lexer->line, .column=lexer->column };
                diag_report(ST_LEXER, SV_ERROR, t, NULL);
                return;
            }
        } else {
            // Carácter con más de un carácter (sin ser escape válido)
            token->type = TOK_ERROR;
            ErrorToken t = { .code=1005, .lexeme=lexer->source+start_pos, .length=len, .line=lexer->line, .column=lexer->column };
            diag_report(ST_LEXER, SV_ERROR, t, NULL);
            return;
        }
    } else {
        // Es un string
        token->type = TOK_STRING;
    }

    lexer->position = i - 1; // Posicionar en la comilla de cierre
    lexer->column += len;
}

static void identify_delimiter(Lexer *lexer, Token *token) { token->type = TOK_DELIMITER; }
static void identify_operator(Lexer *lexer, Token *token)  { token->type = TOK_LOGIC_OP; }
static void identify_error(Lexer *lexer, Token *token)     { token->type = TOK_ERROR; }

// Funciones para saltar comentarios de línea
static void skip_line_comment(Lexer *lexer) {
    lexer->position += 2;
    lexer->column += 2;
    while (lexer->position < lexer->length && lexer->source[lexer->position] != '\n') {
        lexer->position++;
        lexer->column++;
    }
}

// Función para saltar comentarios de bloque
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

//------------Main Lexer Functions----------------

void init_lexer(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->length = strlen(source);
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
}

void tokenize(Lexer *lexer, TokenList *list) {
    Token token; // Declarar una sola variable token reutilizable

    while (lexer->position < lexer->length) {
        char current = lexer->source[lexer->position];

        if (current == '/' && lexer->position + 1 < lexer->length) {
            if (lexer->source[lexer->position + 1] == '/') { skip_line_comment(lexer); continue; }
            if (lexer->source[lexer->position + 1] == '*') { skip_block_comment(lexer); continue; }
        }

        if (is_space(current)) {
            // Saltar espacios en blanco sin crear token
            if (current == '\n') {
                lexer->line++;
                lexer->column = 1;
            } else {
                lexer->column++;
            }
            lexer->position++;
            continue;
        }
        
        init_token(&token); // Inicializar antes de usar

        if (is_alpha(current)) {
            identify_identifier(lexer, &token);
        } else if (is_digit(current)) {
            identify_number(lexer, &token);
        } else if (current == '"' || current == '\'') {
            identify_literal(lexer, &token);
        } else {
            // Caracteres no reconocidos
            token.type = TOK_ERROR;
            token.lexeme = lexer->source + lexer->position;
            token.length = 1;
            token.line = lexer->line;
            token.column = lexer->column;
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