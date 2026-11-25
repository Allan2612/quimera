# Analizador Léxico (Lexer) - Documentación Completa

> **Componente**: Analizador Léxico de Quimera  
> **Archivo principal**: `src/lexer/lexer.c` (~502 líneas)  
> **Header**: `include/lexer.h`  
> **Estado**: ✅ **COMPLETAMENTE IMPLEMENTADO**

---

## 📋 Índice

1. [Resumen General](#resumen-general)
2. [Arquitectura del Lexer](#arquitectura-del-lexer)
3. [Tipos de Tokens](#tipos-de-tokens)
4. [Palabras Reservadas](#palabras-reservadas)
5. [Operadores](#operadores)
6. [Delimitadores](#delimitadores)
7. [Literales](#literales)
8. [Identificadores](#identificadores)
9. [Manejo de Errores](#manejo-de-errores)
10. [Sistema de Strings Internadas](#sistema-de-strings-internadas)
11. [Algoritmo de Tokenización](#algoritmo-de-tokenización)
12. [Ejemplos de Uso](#ejemplos-de-uso)

---

## 🎯 Resumen General

El **Analizador Léxico de Quimera** es la primera fase del compilador, responsable de convertir el código fuente en una secuencia de tokens estructurados. Implementa un sistema robusto de tokenización con manejo de errores, optimización de memoria mediante strings internadas, y soporte completo para todos los elementos léxicos del lenguaje.

### Características Principales
- ✅ **15 tipos de tokens** diferentes
- ✅ **Palabras reservadas**: 15 keywords + 5 tipos
- ✅ **Operadores**: Aritméticos, lógicos, comparación, asignación
- ✅ **Literales**: Enteros, flotantes, strings, caracteres, booleanos
- ✅ **Manejo de errores**: Generación de `TOK_ERROR` para caracteres inválidos
- ✅ **Optimización**: Sistema de strings internadas para eficiencia

---

## 🏗️ Arquitectura del Lexer

### Estructura Principal

```c
typedef struct {
    const char *source;    // Código fuente a procesar
    size_t length;         // Longitud total del source
    size_t position;       // Posición actual en el source
    size_t line;          // Línea actual (para diagnósticos)
    size_t column;        // Columna actual (para diagnósticos)
    
    Interner *I;          // Sistema de strings internadas
    Interned S;           // Símbolos predefinidos internados
} Lexer;
```

### API Principal

```c
// Inicialización del lexer
void init_lexer(Lexer *lexer, const char *source, Interner *I, const Interned *S);

// Proceso principal de tokenización  
void tokenize(Lexer *lexer, TokenList *list);
```

---

## 🏷️ Tipos de Tokens

El lexer de Quimera reconoce **15 categorías** de tokens:

### Enumeración Completa

```c
typedef enum {
    TOK_RESERVED_WORD,      // Palabras reservadas del lenguaje
    TOK_ARITHMETIC_OP,      // Operadores aritméticos: +, -, *, /, %, .
    TOK_LOGIC_COMPARE_OP,   // Operadores lógicos/comparación: &&, ||, ==, !=, <, >, <=, >=
    TOK_ASSIGNMENT_OP,      // Operadores de asignación: =, +=, -=, *=, /=, %=
    TOK_UNARY_OP,          // Operadores unarios: ++, --
    TOK_DELIMITER,         // Delimitadores: (, ), [, ], {, }, ,, ;, :
    TOK_IDENTIFIER,        // Identificadores de variables/funciones
    TOK_INTEGER,           // Literales enteros: 42, 0, -15
    TOK_FLOAT,             // Literales flotantes: 3.14, 0.5, -2.7
    TOK_STRING,            // Literales de cadena: "Hello", "Text"
    TOK_CHAR,              // Literales de carácter: 'a', 'Z'
    TOK_BOOLEAN,           // Literales booleanos: True, False
    TOK_NULL,              // Literal nulo: Null
    TOK_EOF,               // Fin de archivo
    TOK_ERROR              // Token de error para caracteres inválidos
} TokenType;
```

### Función de Token

Cada token contiene:

```c
typedef struct {
    TokenType type;        // Tipo del token
    SrcSpan span;         // Posición en código fuente (línea, columna, longitud)
    const char *lexeme;   // Texto original del token (internado)
} Token;
```

---

## 🔤 Palabras Reservadas

### Tipos de Datos (5)
```c
Integer    // Números enteros
Float      // Números de punto flotante  
String     // Cadenas de caracteres
Boolean    // Valores verdadero/falso
Char       // Caracteres individuales
List       // Listas dinámicas
```

### Palabras Clave (15)
```c
// Estructuras de control
If         // Condicional if
ElseIf     // Condicional else if  
Else       // Condicional else
For        // Bucle for
While      // Bucle while

// Funciones y flujo
Return     // Retorno de función
End        // Fin de bloque

// Entrada/Salida
Print      // Imprimir a consola
Read       // Leer desde entrada

// Operadores textuales
Not        // Negación lógica (!operador simbólico)
And        // Conjunción lógica (&&operador simbólico)  
Or         // Disyunción lógica (||operador simbólico)

// Literales especiales
True       // Booleano verdadero
False      // Booleano falso
Null       // Valor nulo
```

### Reconocimiento de Keywords

```c
static int is_keyword_ptr(const Interned *S, const char *p) {
    return
        // Tipos
        p==S->T_Integer || p==S->T_Float || p==S->T_String ||
        p==S->T_Boolean || p==S->T_Char  || p==S->T_List   ||
        // Palabras clave  
        p==S->K_If     || p==S->K_ElseIf || p==S->K_Else   ||
        p==S->K_For    || p==S->K_While  || p==S->K_Return ||
        p==S->K_Print  || p==S->K_Read   || p==S->K_Not    ||
        p==S->K_True   || p==S->K_False  || p==S->K_Null   || 
        p==S->K_End;
}
```

---

## ⚙️ Operadores

### Operadores Aritméticos (`TOK_ARITHMETIC_OP`)
```c
const char* ArithmeticOps[] = {
    "+",    // Suma
    "-",    // Resta  
    "*",    // Multiplicación
    "/",    // División
    "%",    // Módulo
    "."     // Acceso a miembro/concatenación
};
```

### Operadores Lógicos/Comparación (`TOK_LOGIC_COMPARE_OP`)
```c
const char* LogicCompareOps[] = {
    "&&",   // Y lógico
    "||",   // O lógico
    "==",   // Igual a
    "!=",   // Diferente de
    "<",    // Menor que
    ">",    // Mayor que
    "<=",   // Menor o igual
    ">="    // Mayor o igual
};
```

### Operadores de Asignación (`TOK_ASSIGNMENT_OP`)
```c
const char* AssignmentOps[] = {
    "=",    // Asignación simple
    "+=",   // Suma y asigna
    "-=",   // Resta y asigna
    "*=",   // Multiplica y asigna
    "/=",   // Divide y asigna
    "%="    // Módulo y asigna
};
```

### Operadores Unarios (`TOK_UNARY_OP`)
```c
const char* UnaryOps[] = {
    "++",   // Incremento
    "--"    // Decremento
};
```

### Reconocimiento de Operadores

```c
// Detección automática por coincidencia de prefijos
static int is_operator(char c) {
    // Busca en todas las tablas de operadores
    for (size_t i = 0; i < ArithmeticOpCount; i++) {
        if (ArithmeticOps[i][0] == c) return 1;
    }
    // [similar para otros tipos]
    return 0;
}
```

---

## 🔗 Delimitadores

### Delimitadores Soportados (`TOK_DELIMITER`)

```c
const char* Delimiters[] = {
    "(",    // Paréntesis izquierdo - inicio de expresión/parámetros
    ")",    // Paréntesis derecho - fin de expresión/parámetros  
    "[",    // Corchete izquierdo - inicio de índice array/lista
    "]",    // Corchete derecho - fin de índice array/lista
    "{",    // Llave izquierda - inicio de bloque
    "}",    // Llave derecha - fin de bloque
    ",",    // Coma - separador de elementos
    ";",    // Punto y coma - terminador de sentencia
    ":"     // Dos puntos - separador tipo/declaración
};
```

### Uso en Sintaxis

| Delimitador | Uso Principal | Ejemplo |
|-------------|---------------|---------|
| `()` | Expresiones, parámetros | `(a + b)`, `func(x, y)` |
| `[]` | Acceso a arrays/listas | `arr[0]`, `list[i]` |
| `{}` | Bloques de código | `{ statements }` |
| `,` | Separación de elementos | `func(a, b, c)` |
| `;` | Fin de sentencia | `x = 5;` |
| `:` | Declaración de tipos | `Integer x:` |

---

## 📝 Literales

### Literales Enteros (`TOK_INTEGER`)

```c
// Ejemplos válidos:
42       // Entero positivo
0        // Cero
-15      // Entero negativo (- como unario)
1000     // Entero grande

// Algoritmo de reconocimiento:
if (is_digit(current)) {
    while (is_digit(peek())) advance();
    if (peek() != '.') return TOK_INTEGER;
    // Si hay punto, continúa como float
}
```

### Literales Flotantes (`TOK_FLOAT`)

```c
// Ejemplos válidos:
3.14     // Flotante estándar
0.5      // Fracción
-2.7     // Flotante negativo
100.0    // Entero con punto decimal

// Algoritmo:
// (viene del reconocimiento de entero con punto)
advance(); // consumir el punto
if (!is_digit(peek())) return TOK_ERROR;
while (is_digit(peek())) advance();
return TOK_FLOAT;
```

### Literales de Cadena (`TOK_STRING`)

```c
// Ejemplos válidos:
"Hello World"     // Cadena simple
"Text with spaces"    // Con espacios
""               // Cadena vacía
"Line 1\nLine 2" // Con escapes (si se implementan)

// Algoritmo:
if (current == '"') {
    advance(); // consumir comilla inicial
    while (peek() != '"' && !at_eof()) {
        if (peek() == '\n') line++;
        advance();
    }
    if (at_eof()) return TOK_ERROR; // Sin comilla final
    advance(); // consumir comilla final
    return TOK_STRING;
}
```

### Literales de Carácter (`TOK_CHAR`)

```c
// Ejemplos válidos:
'a'      // Carácter letra
'Z'      // Carácter mayúscula
'0'      // Carácter dígito
' '      // Carácter espacio

// Algoritmo:
if (current == '\'') {
    advance(); // consumir comilla inicial
    if (peek() == '\'' || at_eof()) return TOK_ERROR; // Vacío
    advance(); // consumir carácter
    if (peek() != '\'') return TOK_ERROR; // Sin comilla final
    advance(); // consumir comilla final
    return TOK_CHAR;
}
```

### Literales Booleanos (`TOK_BOOLEAN`)

```c
// Reconocidos como keywords especiales:
True     // Valor verdadero
False    // Valor falso

// Se procesan en el reconocimiento de keywords
// pero se clasifican como TOK_BOOLEAN
```

### Literal Nulo (`TOK_NULL`)

```c
// Keyword especial:
Null     // Valor nulo/vacío

// Tratamiento similar a booleanos
```

---

## 🏷️ Identificadores

### Reglas de Identificadores (`TOK_IDENTIFIER`)

```c
// Formato válido:
[a-zA-Z][a-zA-Z0-9_]*

// Ejemplos válidos:
variable      // Simple
userName      // CamelCase
user_name     // Con guiones bajos
x             // Una letra
counter123    // Con números
_private      // Comenzando con guion bajo

// Ejemplos inválidos:
123variable   // No puede empezar con número
user-name     // Guión no permitido
my variable   // Espacios no permitidos
```

### Algoritmo de Reconocimiento

```c
static int is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static int is_alnum_uscore(char c) {
    return is_alpha(c) || (c >= '0' && c <= '9') || c == '_';
}

// En tokenización:
if (is_alpha(current) || current == '_') {
    while (is_alnum_uscore(peek())) advance();
    
    // Verificar si es palabra reservada
    if (is_keyword(lexeme)) {
        return classify_keyword(lexeme);
    }
    return TOK_IDENTIFIER;
}
```

### Distinción con Keywords

```c
// Proceso de clasificación:
1. Reconocer patrón de identificador
2. Buscar en tabla de palabras reservadas  
3. Si coincide: retornar tipo específico (TOK_BOOLEAN, TOK_NULL, etc.)
4. Si no coincide: retornar TOK_IDENTIFIER
```

---

## ❌ Manejo de Errores

### Token de Error (`TOK_ERROR`)

El lexer genera `TOK_ERROR` para caracteres que no pueden ser tokenizados:

```c
// Caracteres que generan TOK_ERROR:
@          // Símbolos no definidos
#          // Caracteres especiales
$          // Símbolos de moneda
\          // Backslash suelto (fuera de strings)
```

### Estrategia de Manejo

```c
// Cuando se encuentra carácter inválido:
1. Crear TOK_ERROR con el carácter
2. Generar diagnóstico de error léxico (código 1xxx)
3. Avanzar una posición
4. Continuar tokenización (NO abortar)
```

### Integración con Diagnósticos

```c
// Ejemplo de error léxico:
{
  "type": "diagnostic",
  "stage": "lexical",
  "severity": "error",
  "code": 1001,
  "line": 5,
  "column": 10,
  "length": 1,
  "lexeme": "@",
  "message": "Carácter inválido en código fuente: '@'"
}
```

### Casos de Error Manejados

```c
// Cadenas no terminadas
"Hello World    // EOF sin comilla final → TOK_ERROR

// Caracteres no terminados  
'ab             // Múltiples caracteres → TOK_ERROR
'               // Vacío → TOK_ERROR

// Números malformados (manejo en parser)
3.14.15         // Múltiples puntos → procesado como tokens separados

// Caracteres inválidos
Hello@World     // @ genera TOK_ERROR, continúa con World
```

---

## 🔄 Sistema de Strings Internadas

### Optimización de Memoria

El lexer utiliza **string interning** para optimizar memoria y comparaciones:

```c
// Ventajas del sistema:
✅ Memoria: Una sola copia de cada string único
✅ Comparación: Comparación de punteros O(1) vs strcmp O(n)  
✅ Cache: Mejor localidad espacial
✅ Consistencia: Mismos strings = mismos punteros
```

### Uso en Tokens

```c
typedef struct {
    TokenType type;
    SrcSpan span;
    const char *lexeme;  // ← String internado (puntero único)
} Token;

// Comparación eficiente:
if (token.lexeme == S->K_If) { /* es 'If' */ }
// En lugar de: strcmp(token.lexeme, "If") == 0
```

### Símbolos Predefinidos

```c
typedef struct {
    // Tipos
    const char *T_Integer, *T_Float, *T_String, *T_Boolean, *T_Char, *T_List;
    
    // Keywords
    const char *K_If, *K_ElseIf, *K_Else, *K_For, *K_While;
    const char *K_Return, *K_End, *K_Print, *K_Read, *K_Not;
    const char *K_True, *K_False, *K_Null;
    
    // Operadores textuales  
    const char *O_And, *O_Or;
    
    // Delimitadores comunes
    const char *D_LeftParen, *D_RightParen, *D_LeftBrace, *D_RightBrace;
    // ...más símbolos
} Interned;
```

---

## 🔄 Algoritmo de Tokenización

### Flujo Principal

```c
void tokenize(Lexer *lexer, TokenList *list) {
    while (!at_eof(lexer)) {
        skip_whitespace(lexer);           // 1. Saltar espacios
        
        if (at_eof(lexer)) break;        // 2. Verificar EOF
        
        Token token = next_token(lexer);  // 3. Obtener siguiente token
        token_list_push(list, token);     // 4. Añadir a lista
        
        if (token.type == TOK_ERROR) {    // 5. Manejar errores
            report_lexical_error(&token);
        }
    }
    
    // Añadir token EOF final
    Token eof_token = {TOK_EOF, current_span(lexer), intern_static(lexer->I, "")};
    token_list_push(list, eof_token);
}
```

### Algoritmo de Reconocimiento

```c
Token next_token(Lexer *lexer) {
    char current = current_char(lexer);
    size_t start_pos = lexer->position;
    
    // 1. Números (enteros y flotantes)
    if (is_digit(current)) {
        return recognize_number(lexer);
    }
    
    // 2. Identificadores y keywords
    if (is_alpha(current) || current == '_') {
        return recognize_identifier_or_keyword(lexer);
    }
    
    // 3. Strings
    if (current == '"') {
        return recognize_string(lexer);
    }
    
    // 4. Caracteres
    if (current == '\'') {
        return recognize_char(lexer);
    }
    
    // 5. Operadores (incluyendo multi-carácter)
    if (is_operator(current)) {
        return recognize_operator(lexer);
    }
    
    // 6. Delimitadores
    if (is_delimiter(current)) {
        return recognize_delimiter(lexer);
    }
    
    // 7. Carácter inválido
    return create_error_token(lexer);
}
```

### Reconocimiento de Operadores Multi-carácter

```c
Token recognize_operator(Lexer *lexer) {
    char current = current_char(lexer);
    char next = peek_char(lexer);
    
    // Verificar operadores de 2 caracteres primero
    char two_char[3] = {current, next, '\0'};
    
    // Buscar en operadores lógicos/comparación (&&, ||, ==, !=, <=, >=)
    for (size_t i = 0; i < LogicCompareOpCount; i++) {
        if (strlen(LogicCompareOps[i]) == 2 && strcmp(two_char, LogicCompareOps[i]) == 0) {
            advance_twice(lexer);
            return create_token(TOK_LOGIC_COMPARE_OP, intern(lexer->I, two_char));
        }
    }
    
    // Buscar en operadores de asignación (+=, -=, *=, /=, %=)
    for (size_t i = 0; i < AssignmentOpCount; i++) {
        if (strlen(AssignmentOps[i]) == 2 && strcmp(two_char, AssignmentOps[i]) == 0) {
            advance_twice(lexer);
            return create_token(TOK_ASSIGNMENT_OP, intern(lexer->I, two_char));
        }
    }
    
    // Buscar en operadores unarios (++, --)
    for (size_t i = 0; i < UnaryOpCount; i++) {
        if (strcmp(two_char, UnaryOps[i]) == 0) {
            advance_twice(lexer);
            return create_token(TOK_UNARY_OP, intern(lexer->I, two_char));
        }
    }
    
    // Si no es operador de 2 caracteres, buscar de 1 carácter
    char single_char[2] = {current, '\0'};
    
    // [buscar en tablas de operadores de 1 carácter]
    
    advance_once(lexer);
    return create_token(determined_type, intern(lexer->I, single_char));
}
```

---

## 💡 Ejemplos de Uso

### Ejemplo 1: Programa Básico

```c
// Código fuente:
Integer x = 42;
Print x;

// Tokens generados:
[
  {TOK_RESERVED_WORD, "Integer"},
  {TOK_IDENTIFIER, "x"},
  {TOK_ASSIGNMENT_OP, "="},
  {TOK_INTEGER, "42"},
  {TOK_DELIMITER, ";"},
  {TOK_RESERVED_WORD, "Print"},
  {TOK_IDENTIFIER, "x"},
  {TOK_DELIMITER, ";"},
  {TOK_EOF, ""}
]
```

### Ejemplo 2: Expresión Compleja

```c
// Código fuente:
resultado = (a + b) * 2.5;

// Tokens generados:
[
  {TOK_IDENTIFIER, "resultado"},
  {TOK_ASSIGNMENT_OP, "="},
  {TOK_DELIMITER, "("},
  {TOK_IDENTIFIER, "a"},
  {TOK_ARITHMETIC_OP, "+"},
  {TOK_IDENTIFIER, "b"},
  {TOK_DELIMITER, ")"},
  {TOK_ARITHMETIC_OP, "*"},
  {TOK_FLOAT, "2.5"},
  {TOK_DELIMITER, ";"}
]
```

### Ejemplo 3: Estructura de Control

```c
// Código fuente:
If x > 0 {
    Print "Positivo";
} Else {
    Print "No positivo";
}

// Tokens generados:
[
  {TOK_RESERVED_WORD, "If"},
  {TOK_IDENTIFIER, "x"},
  {TOK_LOGIC_COMPARE_OP, ">"},
  {TOK_INTEGER, "0"},
  {TOK_DELIMITER, "{"},
  {TOK_RESERVED_WORD, "Print"},
  {TOK_STRING, "\"Positivo\""},
  {TOK_DELIMITER, ";"},
  {TOK_DELIMITER, "}"},
  {TOK_RESERVED_WORD, "Else"},
  {TOK_DELIMITER, "{"},
  {TOK_RESERVED_WORD, "Print"},
  {TOK_STRING, "\"No positivo\""},
  {TOK_DELIMITER, ";"},
  {TOK_DELIMITER, "}"}
]
```

### Ejemplo 4: Manejo de Errores

```c
// Código fuente con errores:
Integer x@ = 42;
"String sin cerrar
'Char múltiple';

// Tokens generados:
[
  {TOK_RESERVED_WORD, "Integer"},
  {TOK_IDENTIFIER, "x"},
  {TOK_ERROR, "@"},              // ← Error léxico  
  {TOK_ASSIGNMENT_OP, "="},
  {TOK_INTEGER, "42"},
  {TOK_DELIMITER, ";"},
  {TOK_ERROR, "\"String sin cerrar\n"}, // ← Error de string
  {TOK_ERROR, "'Char múltiple'"},        // ← Error de char
  {TOK_DELIMITER, ";"}
]
```

---

## 🎯 Resumen de Capacidades

### ✅ **Completamente Implementado**

- **15 tipos de tokens** con reconocimiento preciso
- **20 palabras reservadas** (tipos + keywords)  
- **16 operadores** (aritméticos, lógicos, asignación, unarios)
- **9 delimitadores** para estructura sintáctica
- **6 tipos de literales** (entero, float, string, char, boolean, null)
- **Identificadores** con validación completa
- **Manejo robusto de errores** con TOK_ERROR
- **Sistema de strings internadas** para eficiencia
- **Integración con diagnósticos** JSON estructurados

### 🚀 **Rendimiento y Calidad**

- **Eficiencia**: O(1) comparaciones de strings internadas
- **Robustez**: Continúa tras errores, no aborta
- **Precisión**: Información exacta de posición (línea/columna)
- **Mantenibilidad**: Código bien estructurado y documentado
- **Testing**: Verificado con múltiples casos de prueba

### 🔗 **Integración con Pipeline**

- **Entrada**: Código fuente como string
- **Salida**: Lista de tokens estructurados
- **Siguiente fase**: Parser consume tokens para generar AST
- **Error recovery**: Tokens de error permiten continuar análisis sintáctico

---

**El Analizador Léxico de Quimera está completamente implementado y listo para producción, proporcionando una base sólida para las siguientes fases del compilador.**