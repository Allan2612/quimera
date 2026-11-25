# Analizador Sintáctico (Parser) - Documentación Completa

> **Componente**: Analizador Sintáctico de Quimera  
> **Archivo principal**: `src/parser/parser.c` (~1028 líneas)  
> **Header**: `include/parser.h`  
> **Estado**: ✅ **COMPLETAMENTE IMPLEMENTADO**

---

## 📋 Índice

1. [Resumen General](#resumen-general)
2. [Arquitectura del Parser](#arquitectura-del-parser)
3. [Gramática del Lenguaje](#gramática-del-lenguaje)
4. [Estructura del AST](#estructura-del-ast)
5. [Análisis de Declaraciones](#análisis-de-declaraciones)
6. [Análisis de Expresiones](#análisis-de-expresiones)
7. [Estructuras de Control](#estructuras-de-control)
8. [Precedencia de Operadores](#precedencia-de-operadores)
9. [Recuperación de Errores](#recuperación-de-errores)
10. [Integración con Sistema de Diagnósticos](#integración-con-sistema-de-diagnósticos)
11. [Ejemplos de Parsing](#ejemplos-de-parsing)

---

## 🎯 Resumen General

El **Analizador Sintáctico de Quimera** es la segunda fase del compilador, responsable de convertir la secuencia de tokens en un **Árbol de Sintaxis Abstracta (AST)**. Implementa un parser **descendente recursivo** con **recuperación de errores** robusta y soporte completo para todas las construcciones sintácticas del lenguaje.

### Características Principales
- ✅ **Parser descendente recursivo** para análisis top-down
- ✅ **AST completo** con 15+ tipos de nodos
- ✅ **Precedencia de operadores** correcta (6 niveles)
- ✅ **Recuperación de errores** con modo pánico
- ✅ **Manejo de TOK_ERROR** específico
- ✅ **Diagnósticos estructurados** con códigos 2000+
- ✅ **Integración robusta** con lexer y analyzer

---

## 🏗️ Arquitectura del Parser

### Estructura Principal

```c
typedef struct {
    const Token *toks;      // Array de tokens del lexer
    size_t count;           // Número total de tokens
    size_t i;              // Índice del token actual
    AstArena *arena;        // Arena para gestión de memoria del AST
    Interner *interner;     // Sistema de strings internadas
    Interned *syms;         // Símbolos predefinidos internados
    int error_count;        // Contador de errores sintácticos
} Parser;
```

### API Principal

```c
// Punto de entrada principal
AstNode *parse_program(Parser *p);

// Helpers para análisis específicos
AstNode *parse_block(Parser *p);    // Bloques de sentencias
AstNode *parse_stmt(Parser *p);     // Sentencias individuales  
AstNode *parse_expr(Parser *p);     // Expresiones
```

### Utilidades de Tokens

```c
// Navegación básica
static inline int is_eof(Parser *p);
static inline const Token *at(Parser *p);           // Token actual
static inline const Token *peek(Parser *p, size_t k); // Token k posiciones adelante
static inline const Token *advance(Parser *p);       // Avanzar y retornar token

// Verificación de tipos
static inline int tok_is(const Token *t, TokenType ty);
static int lexeme_is(Parser *p, const Token *t, const char *s);
static int is_kw(Parser *p, const Token *t, const char *kw);
static int is_delim(Parser *p, const Token *t, const char *d);
static int is_type(Parser *p, const Token *t);
```

---

## 📝 Gramática del Lenguaje

### Gramática BNF de Quimera

```bnf
<program> ::= <statement_list>

<statement_list> ::= <statement>*

<statement> ::= <declaration>
              | <assignment>
              | <expression_statement>
              | <if_statement>
              | <while_statement>
              | <for_statement>
              | <return_statement>
              | <print_statement>
              | <read_statement>
              | <function_definition>
              | <block>

<declaration> ::= <type> <identifier> [':'] [<assignment_op> <expression>] ';'

<type> ::= 'Integer' | 'Float' | 'String' | 'Boolean' | 'Char' | 'List'

<assignment> ::= <identifier> <assignment_op> <expression> ';'

<assignment_op> ::= '=' | '+=' | '-=' | '*=' | '/=' | '%='

<expression_statement> ::= <expression> ';'

<if_statement> ::= 'If' <expression> '{' <statement_list> '}' 
                   ['ElseIf' <expression> '{' <statement_list> '}']* 
                   ['Else' '{' <statement_list> '}']

<while_statement> ::= 'While' <expression> '{' <statement_list> '}'

<for_statement> ::= 'For' '(' <declaration> <expression> ';' <assignment> ')' 
                    '{' <statement_list> '}'

<return_statement> ::= 'Return' [<expression>] ';'

<print_statement> ::= 'Print' <expression> ';'

<read_statement> ::= 'Read' <identifier> ';'

<function_definition> ::= <type> <identifier> '(' <parameter_list> ')' 
                          '{' <statement_list> '}' 'End'

<parameter_list> ::= [<parameter> [',' <parameter>]*]

<parameter> ::= <type> <identifier>

<block> ::= '{' <statement_list> '}'

<expression> ::= <logical_or>

<logical_or> ::= <logical_and> ['||' <logical_and>]*

<logical_and> ::= <equality> ['&&' <equality>]*

<equality> ::= <relational> [('==' | '!=') <relational>]*

<relational> ::= <additive> [('<' | '>' | '<=' | '>=') <additive>]*

<additive> ::= <multiplicative> [('+' | '-') <multiplicative>]*

<multiplicative> ::= <unary> [('*' | '/' | '%') <unary>]*

<unary> ::= ('!' | '-' | '++' | '--') <unary> | <postfix>

<postfix> ::= <primary> [('++' | '--' | '[' <expression> ']' | '(' <argument_list> ')' | '.' <identifier>)]*

<primary> ::= <identifier>
            | <integer_literal>
            | <float_literal>
            | <string_literal>
            | <char_literal>
            | <boolean_literal>
            | <null_literal>
            | '(' <expression> ')'
            | <list_literal>

<argument_list> ::= [<expression> [',' <expression>]*]

<list_literal> ::= '[' [<expression> [',' <expression>]*] ']'
```

---

## 🌳 Estructura del AST

### Tipos de Nodos AST

```c
typedef enum {
    AST_PROGRAM,           // Programa principal
    AST_DECLARATION,       // Declaración de variable
    AST_ASSIGNMENT,        // Asignación
    AST_IF,               // Condicional if
    AST_WHILE,            // Bucle while
    AST_FOR,              // Bucle for
    AST_RETURN,           // Sentencia return
    AST_PRINT,            // Sentencia print
    AST_READ,             // Sentencia read
    AST_FUNCTION_DEF,     // Definición de función
    AST_FUNCTION_CALL,    // Llamada a función
    AST_BINARY_OP,        // Operación binaria
    AST_UNARY_OP,         // Operación unaria
    AST_IDENTIFIER,       // Identificador
    AST_INTEGER_LITERAL,  // Literal entero
    AST_FLOAT_LITERAL,    // Literal flotante
    AST_STRING_LITERAL,   // Literal string
    AST_CHAR_LITERAL,     // Literal char
    AST_BOOLEAN_LITERAL,  // Literal booleano
    AST_NULL_LITERAL,     // Literal null
    AST_LIST_LITERAL,     // Literal de lista
    AST_INDEX_ACCESS,     // Acceso por índice
    AST_MEMBER_ACCESS,    // Acceso a miembro
    AST_BLOCK,            // Bloque de sentencias
    AST_EXPRESSION_STMT,  // Sentencia de expresión
    AST_ERROR             // Nodo de error para recovery
} AstNodeType;
```

### Estructura de Nodo AST

```c
typedef struct AstNode {
    AstNodeType type;      // Tipo del nodo
    SrcSpan span;          // Información de posición en código fuente
    
    union {
        struct { // AST_PROGRAM
            struct AstNode **statements;
            size_t statement_count;
        } program;
        
        struct { // AST_DECLARATION
            const char *type_name;    // Tipo de la variable
            const char *identifier;   // Nombre de la variable
            struct AstNode *initializer; // Valor inicial (opcional)
        } declaration;
        
        struct { // AST_ASSIGNMENT
            const char *identifier;   // Variable a asignar
            const char *operator;     // Operador de asignación
            struct AstNode *value;    // Valor a asignar
        } assignment;
        
        struct { // AST_IF
            struct AstNode *condition;     // Condición
            struct AstNode *then_block;    // Bloque then
            struct AstNode **elseif_conditions; // Condiciones elseif
            struct AstNode **elseif_blocks;     // Bloques elseif
            size_t elseif_count;
            struct AstNode *else_block;    // Bloque else (opcional)
        } if_stmt;
        
        struct { // AST_BINARY_OP
            const char *operator;     // Operador (+, -, *, etc.)
            struct AstNode *left;     // Operando izquierdo
            struct AstNode *right;    // Operando derecho
        } binary_op;
        
        struct { // AST_FUNCTION_CALL
            const char *name;         // Nombre de la función
            struct AstNode **args;    // Argumentos
            size_t arg_count;
        } function_call;
        
        // ... más estructuras para otros tipos
        
        struct { // Literales simples
            const char *value;        // Valor como string
        } literal;
        
        struct { // AST_ERROR
            const char *message;      // Mensaje de error
            TokenType error_token_type; // Tipo del token que causó error
        } error;
    } data;
} AstNode;
```

---

## 📑 Análisis de Declaraciones

### Declaraciones de Variables

```c
// Sintaxis soportada:
Integer x;              // Declaración simple
Float y = 3.14;         // Con inicialización
String name: = "Juan";  // Con dos puntos opcional

// Función de parsing:
static AstNode *parse_declaration(Parser *p) {
    const Token *type_tok = at(p);
    if (!is_type(p, type_tok)) {
        err(p, type_tok, "Se esperaba tipo de dato");
        return create_error_node(p);
    }
    advance(p); // consumir tipo
    
    const Token *id_tok = at(p);
    if (!tok_is(id_tok, TOK_IDENTIFIER)) {
        err(p, id_tok, "Se esperaba identificador");
        return create_error_node(p);
    }
    advance(p); // consumir identificador
    
    // Dos puntos opcional
    if (is_delim(p, at(p), ":")) {
        advance(p);
    }
    
    // Inicialización opcional
    AstNode *initializer = NULL;
    if (is_assignment(p, at(p), "=")) {
        advance(p); // consumir =
        initializer = parse_expression(p);
    }
    
    expect_delim(p, ";");
    
    return create_declaration_node(p, type_tok->lexeme, id_tok->lexeme, initializer);
}
```

### Declaraciones de Funciones

```c
// Sintaxis:
Integer suma(Integer a, Float b) {
    Return a + b;
} End

static AstNode *parse_function_definition(Parser *p) {
    // Tipo de retorno
    const Token *return_type = at(p);
    if (!is_type(p, return_type)) {
        err(p, return_type, "Se esperaba tipo de retorno");
        return create_error_node(p);
    }
    advance(p);
    
    // Nombre de función
    const Token *name_tok = at(p);
    if (!tok_is(name_tok, TOK_IDENTIFIER)) {
        err(p, name_tok, "Se esperaba nombre de función");
        return create_error_node(p);
    }
    advance(p);
    
    // Parámetros
    expect_delim(p, "(");
    AstNode **params = NULL;
    size_t param_count = 0;
    
    if (!is_delim(p, at(p), ")")) {
        params = parse_parameter_list(p, &param_count);
    }
    
    expect_delim(p, ")");
    expect_delim(p, "{");
    
    // Cuerpo de función
    AstNode *body = parse_block(p);
    
    expect_kw(p, "End");
    
    return create_function_node(p, return_type->lexeme, name_tok->lexeme, 
                               params, param_count, body);
}
```

---

## 🔢 Análisis de Expresiones

### Parser de Expresiones con Precedencia

El parser implementa **precedencia de operadores** usando **parsing descendente recursivo**:

```c
// Niveles de precedencia (menor a mayor):
// 1. Lógico OR (||)
// 2. Lógico AND (&&) 
// 3. Igualdad (==, !=)
// 4. Relacional (<, >, <=, >=)
// 5. Aditivo (+, -)
// 6. Multiplicativo (*, /, %)

static AstNode *parse_expression(Parser *p) {
    return parse_logical_or(p);
}

static AstNode *parse_logical_or(Parser *p) {
    AstNode *left = parse_logical_and(p);
    
    while (is_logic_compare(p, at(p), "||")) {
        const Token *op = advance(p);
        AstNode *right = parse_logical_and(p);
        left = create_binary_op_node(p, op->lexeme, left, right);
    }
    
    return left;
}

static AstNode *parse_logical_and(Parser *p) {
    AstNode *left = parse_equality(p);
    
    while (is_logic_compare(p, at(p), "&&")) {
        const Token *op = advance(p);
        AstNode *right = parse_equality(p);
        left = create_binary_op_node(p, op->lexeme, left, right);
    }
    
    return left;
}

static AstNode *parse_equality(Parser *p) {
    AstNode *left = parse_relational(p);
    
    while (is_logic_compare(p, at(p), "==") || is_logic_compare(p, at(p), "!=")) {
        const Token *op = advance(p);
        AstNode *right = parse_relational(p);
        left = create_binary_op_node(p, op->lexeme, left, right);
    }
    
    return left;
}

// ... continúa con relational, additive, multiplicative, unary, primary
```

### Expresiones Primarias

```c
static AstNode *parse_primary(Parser *p) {
    const Token *t = at(p);
    
    // Literales
    if (tok_is(t, TOK_INTEGER)) {
        advance(p);
        return create_integer_literal_node(p, t->lexeme);
    }
    
    if (tok_is(t, TOK_FLOAT)) {
        advance(p);
        return create_float_literal_node(p, t->lexeme);
    }
    
    if (tok_is(t, TOK_STRING)) {
        advance(p);
        return create_string_literal_node(p, t->lexeme);
    }
    
    if (tok_is(t, TOK_CHAR)) {
        advance(p);
        return create_char_literal_node(p, t->lexeme);
    }
    
    if (tok_is(t, TOK_BOOLEAN)) {
        advance(p);
        return create_boolean_literal_node(p, t->lexeme);
    }
    
    if (tok_is(t, TOK_NULL)) {
        advance(p);
        return create_null_literal_node(p);
    }
    
    // Identificadores
    if (tok_is(t, TOK_IDENTIFIER)) {
        advance(p);
        return create_identifier_node(p, t->lexeme);
    }
    
    // Expresiones parentizadas
    if (is_delim(p, t, "(")) {
        advance(p); // consumir '('
        AstNode *expr = parse_expression(p);
        expect_delim(p, ")");
        return expr;
    }
    
    // Lista literal
    if (is_delim(p, t, "[")) {
        return parse_list_literal(p);
    }
    
    err(p, t, "Se esperaba expresión");
    return create_error_node(p);
}
```

### Llamadas a Función y Accesos

```c
static AstNode *parse_postfix(Parser *p) {
    AstNode *expr = parse_primary(p);
    
    while (true) {
        const Token *t = at(p);
        
        // Llamada a función: func(args)
        if (is_delim(p, t, "(")) {
            advance(p); // consumir '('
            
            AstNode **args = NULL;
            size_t arg_count = 0;
            
            if (!is_delim(p, at(p), ")")) {
                args = parse_argument_list(p, &arg_count);
            }
            
            expect_delim(p, ")");
            
            // Convertir identifier a function_call
            if (expr->type == AST_IDENTIFIER) {
                const char *func_name = expr->data.literal.value;
                expr = create_function_call_node(p, func_name, args, arg_count);
            } else {
                err(p, t, "Llamada a función en expresión no válida");
                return create_error_node(p);
            }
        }
        
        // Acceso por índice: expr[index]
        else if (is_delim(p, t, "[")) {
            advance(p); // consumir '['
            AstNode *index = parse_expression(p);
            expect_delim(p, "]");
            
            expr = create_index_access_node(p, expr, index);
        }
        
        // Acceso a miembro: expr.member
        else if (is_arithmetic(p, t, ".")) {
            advance(p); // consumir '.'
            const Token *member = at(p);
            if (!tok_is(member, TOK_IDENTIFIER)) {
                err(p, member, "Se esperaba identificador después de '.'");
                return create_error_node(p);
            }
            advance(p);
            
            expr = create_member_access_node(p, expr, member->lexeme);
        }
        
        else {
            break;
        }
    }
    
    return expr;
}
```

---

## 🔄 Estructuras de Control

### Condicional If

```c
// Sintaxis completa:
If condition { statements } 
ElseIf condition2 { statements }
Else { statements }

static AstNode *parse_if_statement(Parser *p) {
    expect_kw(p, "If");
    
    AstNode *condition = parse_expression(p);
    expect_delim(p, "{");
    AstNode *then_block = parse_block_statements(p);
    expect_delim(p, "}");
    
    // ElseIf clauses
    AstNode **elseif_conditions = NULL;
    AstNode **elseif_blocks = NULL;
    size_t elseif_count = 0;
    
    while (is_kw(p, at(p), "ElseIf")) {
        advance(p); // consumir ElseIf
        
        // Redimensionar arrays
        elseif_conditions = realloc(elseif_conditions, (elseif_count + 1) * sizeof(AstNode*));
        elseif_blocks = realloc(elseif_blocks, (elseif_count + 1) * sizeof(AstNode*));
        
        elseif_conditions[elseif_count] = parse_expression(p);
        expect_delim(p, "{");
        elseif_blocks[elseif_count] = parse_block_statements(p);
        expect_delim(p, "}");
        
        elseif_count++;
    }
    
    // Else clause opcional
    AstNode *else_block = NULL;
    if (is_kw(p, at(p), "Else")) {
        advance(p); // consumir Else
        expect_delim(p, "{");
        else_block = parse_block_statements(p);
        expect_delim(p, "}");
    }
    
    return create_if_node(p, condition, then_block, 
                         elseif_conditions, elseif_blocks, elseif_count,
                         else_block);
}
```

### Bucle While

```c
// Sintaxis:
While condition { statements }

static AstNode *parse_while_statement(Parser *p) {
    expect_kw(p, "While");
    
    AstNode *condition = parse_expression(p);
    expect_delim(p, "{");
    AstNode *body = parse_block_statements(p);
    expect_delim(p, "}");
    
    return create_while_node(p, condition, body);
}
```

### Bucle For

```c
// Sintaxis:
For (Integer i = 0; i < 10; i++) { statements }

static AstNode *parse_for_statement(Parser *p) {
    expect_kw(p, "For");
    expect_delim(p, "(");
    
    // Inicialización
    AstNode *init = parse_declaration(p);
    
    // Condición
    AstNode *condition = parse_expression(p);
    expect_delim(p, ";");
    
    // Actualización
    AstNode *update = parse_assignment(p);
    expect_delim(p, ")");
    
    // Cuerpo
    expect_delim(p, "{");
    AstNode *body = parse_block_statements(p);
    expect_delim(p, "}");
    
    return create_for_node(p, init, condition, update, body);
}
```

---

## ⚖️ Precedencia de Operadores

### Tabla de Precedencia (menor a mayor)

| Nivel | Operadores | Asociatividad | Descripción |
|-------|------------|---------------|-------------|
| 1 | `\|\|` | Izquierda | O lógico |
| 2 | `&&` | Izquierda | Y lógico |
| 3 | `==`, `!=` | Izquierda | Igualdad |
| 4 | `<`, `>`, `<=`, `>=` | Izquierda | Relacional |
| 5 | `+`, `-` | Izquierda | Aditivo |
| 6 | `*`, `/`, `%` | Izquierda | Multiplicativo |
| 7 | `!`, `-`, `++`, `--` (prefijo) | Derecha | Unario |
| 8 | `++`, `--` (postfijo), `[]`, `()`, `.` | Izquierda | Postfijo |

### Implementación de Precedencia

```c
// El parser implementa precedencia mediante funciones recursivas:

parse_expression()     → parse_logical_or()      // Precedencia 1
parse_logical_or()     → parse_logical_and()     // Precedencia 2  
parse_logical_and()    → parse_equality()        // Precedencia 3
parse_equality()       → parse_relational()      // Precedencia 4
parse_relational()     → parse_additive()        // Precedencia 5
parse_additive()       → parse_multiplicative()  // Precedencia 6
parse_multiplicative() → parse_unary()           // Precedencia 7
parse_unary()          → parse_postfix()         // Precedencia 8
parse_postfix()        → parse_primary()         // Elementos básicos
```

### Ejemplo de Precedencia

```c
// Código fuente:
a + b * c && d == e

// Árbol AST resultante (precedencia aplicada):
        &&
       /  \
      +    ==
     / \   / \
    a   *  d  e
       / \
      b   c

// Evaluación: (a + (b * c)) && (d == e)
```

---

## 🛡️ Recuperación de Errores

### Estrategia de Panic Mode

El parser implementa **panic mode recovery** para continuar el análisis tras errores:

```c
static void panic_mode_recovery(Parser *p) {
    // Buscar tokens de sincronización para recuperarse
    while (!is_eof(p)) {
        const Token *t = at(p);
        
        // Puntos de sincronización comunes
        if (is_delim(p, t, ";") ||      // Fin de sentencia
            is_delim(p, t, "}") ||      // Fin de bloque
            is_kw(p, t, "If") ||        // Inicio de condicional
            is_kw(p, t, "While") ||     // Inicio de bucle
            is_kw(p, t, "For") ||       // Inicio de bucle
            is_kw(p, t, "Return") ||    // Sentencia return
            is_type(p, t)) {            // Inicio de declaración
            break;
        }
        
        advance(p); // Saltar token problemático
    }
    
    // Si encontramos ';', lo consumimos
    if (is_delim(p, at(p), ";")) {
        advance(p);
    }
}
```

### Manejo de TOK_ERROR

```c
static AstNode *handle_token_error(Parser *p) {
    const Token *error_tok = at(p);
    
    if (tok_is(error_tok, TOK_ERROR)) {
        // Crear diagnóstico específico para token de error léxico
        DiagLocation loc = diag_location_create(
            error_tok->line, error_tok->column, 
            error_tok->length, error_tok->lexeme
        );
        diag_error(STAGE_PARSER, 2003, loc, 
                  "Token de error léxico encontrado durante análisis sintáctico");
        
        ++p->error_count;
        advance(p); // Consumir token de error
        
        // Intentar recuperación
        panic_mode_recovery(p);
        
        return create_error_node(p);
    }
    
    return NULL; // No es TOK_ERROR
}
```

### Nodos AST de Error

```c
// Estructura para nodos de error
typedef struct {
    const char *message;          // Mensaje descriptivo del error
    TokenType error_token_type;   // Tipo del token que causó error
    SrcSpan error_span;          // Ubicación del error
} AstErrorNode;

static AstNode *create_error_node(Parser *p) {
    AstNode *node = ast_alloc(p->arena, sizeof(AstNode));
    node->type = AST_ERROR;
    node->span = current_span(p);
    node->data.error.message = "Error sintáctico";
    node->data.error.error_token_type = at(p) ? at(p)->type : TOK_EOF;
    return node;
}
```

### Continuación tras Errores

```c
// El parser continúa procesando incluso con errores:
static AstNode *parse_statement_list(Parser *p) {
    AstNode **statements = NULL;
    size_t count = 0;
    size_t capacity = 0;
    
    while (!is_eof(p) && !is_kw(p, at(p), "End") && !is_delim(p, at(p), "}")) {
        // Manejar TOK_ERROR específicamente
        AstNode *error_node = handle_token_error(p);
        if (error_node) {
            add_statement(&statements, &count, &capacity, error_node);
            continue; // Continuar con siguiente token
        }
        
        // Parsear sentencia normal
        AstNode *stmt = parse_statement(p);
        if (stmt) {
            add_statement(&statements, &count, &capacity, stmt);
        } else {
            // Error en parsing, recuperarse y continuar
            panic_mode_recovery(p);
        }
    }
    
    return create_block_node(p, statements, count);
}
```

---

## 🔧 Integración con Sistema de Diagnósticos

### Códigos de Error del Parser (2000-2999)

```c
#define PARSER_ERROR_UNEXPECTED_TOKEN    2002  // Token inesperado
#define PARSER_ERROR_TOK_ERROR          2003  // TOK_ERROR del lexer
#define PARSER_ERROR_MISSING_SEMICOLON  2004  // Punto y coma faltante
#define PARSER_ERROR_MISSING_BRACE      2005  // Llave faltante
#define PARSER_ERROR_MISSING_PAREN      2006  // Paréntesis faltante
#define PARSER_ERROR_INVALID_EXPRESSION 2007  // Expresión inválida
#define PARSER_ERROR_MISSING_IDENTIFIER 2008  // Identificador esperado
#define PARSER_ERROR_MISSING_KEYWORD    2009  // Palabra clave esperada
#define PARSER_ERROR_MISSING_DELIMITER  2010  // Delimitador esperado
```

### Generación de Diagnósticos

```c
static void report_syntax_error(Parser *p, const Token *t, int code, const char *message) {
    ++p->error_count;
    
    if (t) {
        DiagLocation loc = diag_location_create(
            t->line, t->column, t->length, t->lexeme
        );
        diag_error(STAGE_PARSER, code, loc, message);
    } else {
        DiagLocation loc = diag_location_simple(0, 0);
        diag_error(STAGE_PARSER, code, loc, message);
    }
}

// Ejemplo de uso:
static AstNode *parse_assignment(Parser *p) {
    const Token *id_tok = at(p);
    if (!tok_is(id_tok, TOK_IDENTIFIER)) {
        report_syntax_error(p, id_tok, PARSER_ERROR_MISSING_IDENTIFIER,
                          "Se esperaba identificador en asignación");
        return create_error_node(p);
    }
    
    advance(p);
    
    const Token *op_tok = at(p);
    if (!tok_is(op_tok, TOK_ASSIGNMENT_OP)) {
        report_syntax_error(p, op_tok, PARSER_ERROR_UNEXPECTED_TOKEN,
                          "Se esperaba operador de asignación");
        return create_error_node(p);
    }
    
    // ... continuar parsing
}
```

### Ejemplo de Diagnósticos JSON

```json
{
  "type": "diagnostic",
  "stage": "parser",
  "severity": "error",
  "code": 2009,
  "line": 15,
  "column": 8,
  "length": 3,
  "lexeme": "xyz",
  "message": "Se esperaba palabra clave 'End' al final de función"
}
```

---

## 💡 Ejemplos de Parsing

### Ejemplo 1: Declaración Simple

```c
// Código fuente:
Integer x = 42;

// Tokens de entrada:
[TOK_RESERVED_WORD:"Integer", TOK_IDENTIFIER:"x", 
 TOK_ASSIGNMENT_OP:"=", TOK_INTEGER:"42", TOK_DELIMITER:";"]

// AST resultante:
AstNode {
    type: AST_DECLARATION,
    data.declaration: {
        type_name: "Integer",
        identifier: "x",
        initializer: AstNode {
            type: AST_INTEGER_LITERAL,
            data.literal.value: "42"
        }
    }
}
```

### Ejemplo 2: Expresión Compleja

```c
// Código fuente:
resultado = (a + b) * c;

// AST resultante:
AstNode {
    type: AST_ASSIGNMENT,
    data.assignment: {
        identifier: "resultado",
        operator: "=",
        value: AstNode {
            type: AST_BINARY_OP,
            data.binary_op: {
                operator: "*",
                left: AstNode {
                    type: AST_BINARY_OP,
                    data.binary_op: {
                        operator: "+",
                        left: AstNode {type: AST_IDENTIFIER, value: "a"},
                        right: AstNode {type: AST_IDENTIFIER, value: "b"}
                    }
                },
                right: AstNode {type: AST_IDENTIFIER, value: "c"}
            }
        }
    }
}
```

### Ejemplo 3: Estructura de Control

```c
// Código fuente:
If x > 0 {
    Print "Positivo";
} Else {
    Print "No positivo";
}

// AST resultante:
AstNode {
    type: AST_IF,
    data.if_stmt: {
        condition: AstNode {
            type: AST_BINARY_OP,
            data.binary_op: {
                operator: ">",
                left: AstNode {type: AST_IDENTIFIER, value: "x"},
                right: AstNode {type: AST_INTEGER_LITERAL, value: "0"}
            }
        },
        then_block: AstNode {
            type: AST_BLOCK,
            statements: [
                AstNode {
                    type: AST_PRINT,
                    data.print: {
                        expression: AstNode {
                            type: AST_STRING_LITERAL, 
                            value: "\"Positivo\""
                        }
                    }
                }
            ]
        },
        else_block: AstNode {
            type: AST_BLOCK,
            statements: [
                AstNode {
                    type: AST_PRINT,
                    data.print: {
                        expression: AstNode {
                            type: AST_STRING_LITERAL,
                            value: "\"No positivo\""
                        }
                    }
                }
            ]
        }
    }
}
```

### Ejemplo 4: Manejo de Errores

```c
// Código fuente con errores:
Integer x = ;  // Valor faltante
y + = 5;       // Identificador sin declarar, sintaxis incorrecta

// Parsing con recovery:
1. Parsear "Integer x ="
2. Encontrar error en expresión faltante  
3. Generar nodo AST_ERROR
4. Aplicar panic_mode_recovery hasta ';'
5. Continuar con siguiente sentencia
6. Encontrar error en "y +" (sintaxis inválida)
7. Generar otro nodo AST_ERROR
8. Continuar parsing...

// AST resultante con nodos de error:
[
    AstNode {type: AST_ERROR, message: "Expresión esperada"},
    AstNode {type: AST_ERROR, message: "Sintaxis de asignación inválida"}
]
```

---

## 🎯 Resumen de Capacidades

### ✅ **Completamente Implementado**

- **Parser descendente recursivo** con análisis top-down
- **AST completo** con 25+ tipos de nodos
- **Precedencia correcta** de operadores (6 niveles)
- **Estructuras de control** completas (if/elseif/else, while, for)
- **Declaraciones** de variables y funciones
- **Expresiones complejas** con parentización
- **Llamadas a función** con argumentos
- **Acceso a arrays/listas** por índice
- **Recuperación robusta** de errores sintácticos
- **Manejo específico** de TOK_ERROR del lexer
- **Diagnósticos estructurados** con códigos 2000+

### 🚀 **Rendimiento y Calidad**

- **Eficiencia**: O(n) tiempo de parsing lineal
- **Robustez**: Continúa tras errores, no aborta
- **Precisión**: AST con información completa de posición
- **Mantenibilidad**: Código modular bien estructurado  
- **Testing**: Verificado con casos complejos y erróneos

### 🔗 **Integración con Pipeline**

- **Entrada**: Lista de tokens del lexer
- **Salida**: AST para el analizador semántico
- **Error recovery**: Permite análisis semántico parcial
- **Diagnósticos**: JSON estructurado integrado con sistema unificado

---

**El Analizador Sintáctico de Quimera está completamente implementado y proporciona una base robusta para el análisis semántico, con manejo excepcional de errores y recuperación inteligente.**