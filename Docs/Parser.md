# Parser de Quimera - Documentación Técnica Actualizada

## Introducción

El parser de Quimera es un analizador sintáctico descendente recursivo que implementa el algoritmo de Pratt para el análisis de expresiones. **Está completamente integrado con el sistema de strings internadas (interner)** para optimización de memoria y rendimiento.

## Arquitectura

### Integración Lexer-Parser

#### Estructura del Parser (Actualizada)
```c
typedef struct {
    const Token *toks;    // Array de tokens del lexer
    size_t count;         // Número total de tokens
    size_t i;             // Índice actual (posición en los tokens)
    AstArena *arena;      // Arena de memoria para nodos AST
    Interner *interner;   // Sistema de strings internadas
    Interned *syms;       // Símbolos precargados internados
    int error_count;      // Contador de errores encontrados
} Parser;
```

#### Sistema de Token Check Optimizado
El parser utiliza funciones optimizadas que aprovechan las strings internadas:

```c
// Funciones principales (solo interner)
static int is_kw_intern(Parser *p, const Token *t, const char *interned_kw);
static int is_delim_intern(Parser *p, const Token *t, const char *interned_delim);
static int is_assignment_intern(Parser *p, const Token *t, const char *interned_op);

// Funciones auxiliares para compatibilidad (temporal)
static int is_kw(const Token *t, const char *kw);
static int is_delim(const Token *t, const char *d);
```

### Categorización Consistente de Tokens

#### Operador de Asignación
- **`=`, `+=`, `-=`** → `TOK_ASSIGNMENT_OP` (NO `TOK_DELIMITER`)
- **Parser busca correctamente**: `tok_is(at(p), TOK_ASSIGNMENT_OP) && lexeme_is(at(p), "=")`

#### Operador de Acceso (Punto)
- **`.`** → `TOK_ARITHMETIC_OP` (para acceso a miembros/métodos)
- **Parser busca**: `tok_is(t, TOK_ARITHMETIC_OP) && lexeme_is(t, ".")`

#### Delimitadores
- **`;`, `:`, `{`, `}`, `,`, `(`, `)`, `[`, `]`** → `TOK_DELIMITER`

## Funciones Principales

### Punto de Entrada

- **`parse_program(Parser *p)`**: Función principal que parsea un programa completo
  - Entrada: Parser inicializado con tokens
  - Salida: AST_PROGRAM node conteniendo todas las declaraciones/sentencias

### Parsers de Declaraciones

- **`parse_var_declaration(Parser *p)`**: Variables simples (`tipo nombre;` o `tipo nombre = valor;`)
- **`parse_array_declaration(Parser *p)`**: Arrays (`tipo nombre[tamaño];`)
- **`parse_list_declaration(Parser *p)`**: Listas (`tipo List nombre;`)
- **`parse_function_declaration(Parser *p)`**: Funciones (`tipo nombre(params) ... End`)

### Parsers de Sentencias

- **`parse_if(Parser *p)`**: Sentencias If/ElseIf/Else
- **`parse_while(Parser *p)`**: Ciclos While
- **`parse_for_stmt(Parser *p)`**: Ciclos For
- **`parse_print(Parser *p)`**: Sentencias Print
- **`parse_read(Parser *p)`**: Sentencias Read
- **`parse_return(Parser *p)`**: Sentencias Return

### Parsers de Expresiones (Algoritmo de Pratt)

- **`parse_expression(Parser *p)`**: Punto de entrada para expresiones
- **`parse_precedence(Parser *p, int min_prec)`**: Núcleo del algoritmo de Pratt
- **`parse_prefix(Parser *p)`**: Operadores prefijos (unarios, paréntesis, literales)
- **`parse_postfix(Parser *p, AstNode *lhs)`**: Operadores postfijos (llamadas, indexación)

## Tabla de Precedencias

El parser utiliza la siguiente tabla de precedencias (de mayor a menor):

| Nivel | Operadores | Asociatividad | Descripción |
|-------|------------|---------------|-------------|
| 8 | `()`, `[]`, `.método()` | Izquierda | Llamadas, indexación, métodos |
| 7 | `**` | Derecha | Exponenciación |
| 6 | `+`, `-`, `Not` (unarios) | - | Operadores unarios |
| 5 | `*`, `/`, `%` | Izquierda | Multiplicativos |
| 4 | `+`, `-` (binarios) | Izquierda | Aditivos |
| 3 | `<`, `<=`, `>`, `>=` | Izquierda | Comparadores |
| 2 | `==`, `!=` | Izquierda | Igualdad |
| 1 | `And`, `Or` | Izquierda | Lógicos |
| 0 | `=`, `+=`, `-=` | Derecha | Asignación |

## Algoritmo de Pratt (Precedence Climbing)

### Implementación

El algoritmo de Pratt se implementa en `parse_precedence()` siguiendo estos pasos:

1. **Parse Prefix**: Analizar operadores prefijos y operandos
2. **Parse Postfix**: Aplicar operadores postfijos al resultado
3. **Climbing Loop**: Mientras haya operadores infijos con precedencia suficiente:
   - Determinar asociatividad (izquierda/derecha)
   - Parsear lado derecho recursivamente
   - Construir nodo AST binario

### Ventajas

- **Extensibilidad**: Fácil agregar nuevos operadores modificando la tabla
- **Eficiencia**: O(n) en el número de tokens
- **Claridad**: Código más limpio que parser recursivo tradicional para expresiones

## Manejo de Errores

### Sistema de Diagnóstico

El parser está integrado con el sistema de diagnóstico (`diag_errors.h`):

```c
static void err(Parser *p, const Token *t, const char *msg) {
    ++p->error_count;
    if (t) {
        ErrorToken et = {
            .code = 2001,
            .lexeme = t->lexeme,
            .length = t->length,
            .line = t->line,
            .column = t->column
        };
        diag_report(ST_PARSER, SV_ERROR, et, msg);
    }
}
```

### Estrategias de Recuperación

1. **Modo Pánico Simple**: Avanzar tokens hasta encontrar punto de sincronización
2. **Expect Functions**: Funciones que esperan tokens específicos y reportan errores
3. **Continuación**: El parser intenta continuar después de errores para encontrar más

### Códigos de Error

- **2001**: Error genérico de parser con token válido
- **2002**: Error de parser sin token (EOF inesperado)

## Construcción del AST

### Arena de Memoria

El parser utiliza un `AstArena` para gestión eficiente de memoria:

- **Bump Allocation**: Asignación secuencial rápida
- **No Fragmentación**: Memoria contigua
- **Liberación Simple**: Una sola operación libera todo

### Tipos de Nodos AST

#### Declaraciones
- `AST_VAR_DECL`: Variables, arrays, listas
- `AST_FUNC_DECL`: Funciones
- `AST_PARAM`: Parámetros de función

#### Sentencias
- `AST_IF`, `AST_WHILE`, `AST_FOR`: Control de flujo
- `AST_PRINT`, `AST_READ`, `AST_RETURN`: I/O y retorno
- `AST_EXPR`: Wrapper para expresiones como sentencias

#### Expresiones
- `AST_BINOP`, `AST_UNOP`: Operaciones binarias y unarias
- `AST_ASSIGN`: Asignaciones
- `AST_CALL`: Llamadas a función
- `AST_INDEX`: Indexación de arrays/listas
- `AST_IDENT`: Identificadores
- `AST_LITERAL`: Literales (int, float, string, bool, null)

#### Estructura
- `AST_PROGRAM`: Raíz del programa
- `AST_BLOCK`: Bloques de código

## Características Avanzadas

### Métodos de Lista

El parser soporta métodos especiales para listas:

```c
// lista.append(valor)
// lista.remove(índice)  
// lista.size()
```

Estos se transforman en llamadas a función especiales donde el primer argumento es el objeto lista.

### Declaraciones Contextuales

El parser diferencia entre tipos de declaraciones basándose en lookahead:

- `tipo nombre;` → Variable simple
- `tipo nombre[expr];` → Array
- `tipo List nombre;` → Lista
- `tipo nombre(params)` → Función

### For Loop Flexible

El parser soporta dos tipos de inicialización en bucles for:

- **Declaración**: `For (Integer i = 0 : ...)`
- **Asignación**: `For (i = 0 : ...)`

## Uso del Parser

### Inicialización

```c
// 1. Tokenizar código fuente
Lexer lexer;
TokenList tokens;
init_lexer(&lexer, source_code);
init_token_list(&tokens);
tokenize(&lexer, &tokens);

// 2. Crear arena de memoria
AstArena *arena = ast_arena_new();

// 3. Inicializar parser
Parser parser = {
    .toks = tokens.tokens,
    .count = tokens.count,
    .i = 0,
    .arena = arena,
    .error_count = 0
};

// 4. Parsear programa
AstNode *ast = parse_program(&parser);

// 5. Verificar errores
if (parser.error_count == 0) {
    // AST válido
} else {
    // Hubo errores de sintaxis
}

// 6. Limpiar memoria
ast_arena_free(arena);
free_token_list(&tokens);
```

### Parsers Auxiliares

Para testing o análisis parcial, están disponibles:

```c
AstNode *parse_block(Parser *p);   // Parsear bloque hasta 'End'
AstNode *parse_stmt(Parser *p);    // Parsear una sentencia
AstNode *parse_expr(Parser *p);    // Parsear una expresión
```

## Extensibilidad

### Agregar Nuevos Operadores

1. **Actualizar `OpCode`** en `ast.h`
2. **Modificar `opcode_from_tok()`** para reconocer el token
3. **Actualizar `infix_prec_from_tok()`** para establecer precedencia
4. **Agregar al lexer** si es necesario

### Agregar Nuevas Construcciones

1. **Definir nuevo `AstKind`** en `ast.h`
2. **Extender union en `AstNode`** para campos específicos
3. **Crear función `parse_nueva_construccion()`**
4. **Integrar en `parse_statement()` o `parse_declaration()`**

## Pruebas

Las pruebas están organizadas en:

- **`parser_tests.c`**: Tests de declaraciones y expresiones
- **`statements_tests.c`**: Tests de sentencias y manejo de errores
- **`test_runner.c`**: Registro de todos los tests
- **`test_main.c`**: Punto de entrada para ejecución

### Ejecutar Pruebas

```bash
make clean
make debug
./bin/tests/parser/test_main
```

## Consideraciones de Rendimiento

### Complejidad

- **Temporal**: O(n) donde n es el número de tokens
- **Espacial**: O(d) donde d es la profundidad máxima de anidamiento

### Optimizaciones

- **Arena Allocation**: Elimina overhead de malloc/free individuales
- **Lookahead Limitado**: Máximo 2-3 tokens de lookahead
- **Inline Functions**: Operaciones de tokens marcadas como inline

## Limitaciones Conocidas

1. **Recuperación de Errores**: Estrategia simple de modo pánico
2. **Memoria**: No hay liberación parcial del AST
3. **Unicode**: Soporte limitado para caracteres especiales
4. **Precedencia**: Tabla fija, no configurable en runtime

## Futuras Mejoras

1. **Error Recovery**: Implementar estrategias más sofisticadas
2. **Source Maps**: Mejor mapeo entre AST y código fuente
3. **Incremental Parsing**: Soporte para re-parsing parcial
4. **Memory Pools**: Optimización adicional de memoria
5. **Parallel Parsing**: Análisis paralelo para archivos grandes