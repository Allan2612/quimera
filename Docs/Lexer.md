# Quimera — Léxico (Documentación Actualizada)

> Este documento describe la implementación actual del analizador léxico del lenguaje Quimera,
> incluyendo categorización de tokens, integración con el sistema de strings internadas (interner),
> y el puente lexer-parser.

---

## 1) Integración con Sistema Interner

### Sistema de Strings Internadas
- **Todas las strings del lexer se internalizan** usando FNV-1a hashing
- **Comparación O(1)** por punteros en lugar de O(n) por contenido
- **Gestión automática de memoria** a través de AstArena
- **Símbolos precargados** para palabras reservadas y operadores comunes

### API de Interner
```c
const char *interner_intern_slice(Interner *I, const char *s, size_t n);
const char *interner_intern_cstr(Interner *I, const char *s);
```

## 2) Categorización de Tokens

### Tipos de Token Principales
```c
typedef enum {
    TOK_IDENTIFIER,    // Variables, funciones
    TOK_KEYWORD,       // Palabras reservadas
    TOK_INTEGER,       // Números enteros
    TOK_FLOAT,         // Números de punto flotante
    TOK_ARITHMETIC_OP, // +, -, *, /, %, **, . (acceso)
    TOK_LOGIC_OP,      // ==, !=, >, <, >=, <=, &&, ||, !
    TOK_ASSIGNMENT_OP, // =, +=, -=
    TOK_STRING,        // "cadenas"
    TOK_CHAR,          // 'c'
    TOK_BOOLEAN,       // true, false
    TOK_DELIMITER,     // ;, :, {, }, ,, (, ), [, ]
    TOK_ERROR          // Tokens inválidos
} TokenType;
```

## 3) Palabras Reservadas y Tipos
### Palabras Reservadas (internadas)
- **Tipos de datos**: `Integer`, `Float`, `String`, `Boolean`, `Char`
- **Valores literales**: `True`, `False`, `Null`
- **Control de flujo**: `If`, `ElseIf`, `Else`, `For`, `While`, `Return`, `End`
- **Entrada/Salida**: `Print`, `Read`
- **Operadores lógicos**: `And`, `Or`, `Not`

### Operadores por Categoría

#### Operadores Aritméticos (TOK_ARITHMETIC_OP)
```c
const char * const ArithmeticOps[] = {"+", "-", "*", "/", "%", "**", "."};
```
- **Nota**: El punto `.` se usa para acceso a miembros/métodos

#### Operadores Lógicos/Comparación (TOK_LOGIC_OP)
```c
const char * const LogicCompareOps[] = {"==", "!=", ">", "<", ">=", "<=", "&&", "||", "!"};
```

#### Operadores de Asignación (TOK_ASSIGNMENT_OP)
```c
const char * const AssignmentOps[] = {"=", "+=", "-="};
```
- **IMPORTANTE**: `=` es `TOK_ASSIGNMENT_OP`, no `TOK_DELIMITER`

#### Delimitadores (TOK_DELIMITER)
```c
const char * const Delimiters[] = {";", ":", "{", "}", ",", "(", ")", "[", "]"};
```

## 4) Identificadores
**Reglas de formación** (resumen textual de la documentación):
- Deben **iniciar con letra** (A–Z | a–z).  
- Luego pueden contener **letras, dígitos y `_`** (subrayado).  
- **No** pueden iniciar con dígito ni contener otros símbolos especiales.  
- **No** pueden coincidir con palabras reservadas (p. ej., `Integer`, `If`, `While`).

La documentación incluye ejemplos válidos/ inválidos (p.ej., `contador`, `nombre_usuario`, `_temporal1` vs. `1variable`, `precio$`, `While`).

---

## 4) Literales
El lenguaje soporta los siguientes **tipos de literales** (según el texto y el AFD de literales):
- **Enteros** (positivos/negativos, sin parte decimal): `0, 25, -13`  
- **Flotantes** (con punto decimal): `3.14, -0.5, 10.0`  
- **Caracteres** (comillas simples): `'a', 'Z', '9', '%'`  
- **Cadenas** (comillas dobles): `"Hola", "123", "Variable_x"`  
- **Booleanos**: `True`, `False`  
- **Nulo**: `Null`

---

## 5) Comentarios
- **De una línea** con `#` (hasta fin de línea).  
- **Multilínea** con `/* ... */` (no anidados).  
La documentación provee además el **AFD de comentarios** y su tabla de transición.

---

## 6) Operadores y delimitadores
La documentación presenta dos frentes complementarios:

**a) Cuadros/tablas de operadores lógicos/de comparación y de asignación** (texto):  
- Comparación: `==`, `!=`, `>`, `<`, `>=`, `<=`  
- Lógicos: `&&`, `||`, `!`  
- Asignación: `=`, `+=`, `-=`

**b) AFD de operadores y delimitadores** con conjuntos de entrada:  
`DL = {; , :, (, ), {, }, [, ], , }`, `OA = {+, −, /, %}`, `AS = {∗}`, `EQ = {=}`, `NOT = {!}`, `GT = {>}`, `LT = {<}`, `AMP = {&}`, `BAR = {|}`, y combinaciones para `OL`. Incluye estados para formas compuestas (`==`, `!=`, `>=`, `<=`, `&&`, `||`, `**`/`*`).

**c) Reservadas lógicas en texto**: además del uso simbólico (`&&`, `||`, `!`), el **Cuadro 1** lista `And`, `Or`, `Not` como palabras reservadas. La documentación de léxico recoge **ambas notaciones**.

> Para mantener fidelidad con la documentación, el lexer debe reconocer:  
> • **Palabras**: `And`, `Or`, `Not` (reservadas)  
> • **Símbolos**: `&&`, `||`, `!` (operadores lógicos)

---

## 7) Nota de contexto (precedencia)
Aunque es tema de **sintaxis/semántica**, la documentación incluye una **tabla de precedencia** y asociatividad (p. ej., `() []` > `Not` > `* / %` > `+ -` > comparaciones > `== !=` > `And` > `Or` > `= += -=`). Esta referencia **no** cambia el léxico, pero explica por qué coexisten formas simbólicas y verbales.

---

## 8) Autómatas (AFD) en la documentación
- **Identificadores**: AFD con alfabetos `L`, `D`, `_` y estado pozo `X`.【DOC-ID-AFD】
- **Literales (cadenas)**: AFD con estados `C1, C2, C3` y alfabeto particionado (`S = {\"}`, `O = Σ − S`).
- **Comentarios**: AFD con `#`, `/* ... */`, fin de línea `\n`, y estado de aceptación final.
- **Operadores/Delimitadores**: AFD que cubre `DL, OA, AS, EQ, NOT, GT, LT, AMP, BAR` y sus combinaciones (incluye transiciones a `X`).

---

## 9) Errores léxicos (en la doc)
El sistema de reporte ilustra códigos y ubicación exacta (línea/columna). Ejemplo: **cadena sin cierre** reportada como error del lexer.

---

### Referencias (marcadores internos a la documentación)
-  Sensibilidad a mayúsculas/minúsculas y uso de `#`/`/* */`.  
-  Cuadro de **palabras reservadas** (listado completo).  
-  Reglas de **identificadores** y ejemplos válidos/invalidos.  
-  Descripción textual de **literales**.  
-  **AFD de literales** (tabla/diagrama).  
-  Texto que define **comentarios** `#` y `/* ... */`.  
-  **AFD de comentarios** (tabla/diagrama).  
-  **Tablas** de operadores de comparación/lógicos y de asignación.  
-  **AFD de operadores y delimitadores** (conjuntos y transiciones).  
-  Descripción general de uso de **AFD** y estado pozo `X`.  
-  **Precedencia** (tabla y notas).  
-  Ejemplo de **mensaje de error** de lexer (`[LEXER:1002]`).

