# Integración Lexer-Parser en Quimera

## Resumen

Esta documentación describe cómo se integran el analizador léxico y sintáctico en el lenguaje Quimera, incluyendo la resolución de problemas de categorización de tokens y la migración completa al sistema de strings internadas.

## Problemas Resueltos

### 1. Categorización Inconsistente de Tokens

#### Problema Identificado
- **Operador `=`**: El lexer lo clasificaba como `TOK_ASSIGNMENT_OP`, pero el parser a veces lo buscaba como `TOK_DELIMITER`
- **Operador `.`**: No estaba definido en las tablas de símbolos, causando confusión en la categorización

#### Solución Implementada
```c
// ANTES (problemático)
if (is_delim(at(p), "=")) { ... }

// AHORA (correcto)
if (tok_is(at(p), TOK_ASSIGNMENT_OP) && lexeme_is(at(p), "=")) { ... }

// ANTES (indefinido)
if (is_delim(t, ".")) { ... }

// AHORA (correcto)
if (tok_is(t, TOK_ARITHMETIC_OP) && lexeme_is(t, ".")) { ... }
```

### 2. Sistema de Strings Mixto

#### Problema Identificado
- Funciones duplicadas: `is_kw()` y `is_kw_intern()`
- Uso inconsistente entre comparación de strings y punteros internados
- Pérdida de rendimiento por comparaciones O(n) innecesarias

#### Solución Implementada
```c
// Funciones principales (optimizadas)
static int is_kw_intern(Parser *p, const Token *t, const char *interned_kw) {
    return tok_is(t, TOK_KEYWORD) && lexeme_is_intern(p, t, interned_kw);
}

// Helper para comparación O(1)
static int lexeme_is_intern(Parser *p, const Token *t, const char *interned_str) {
    if (!t || !t->lexeme || !interned_str) return 0;
    const char *interned_lexeme = intern_token_lexeme(p, t);
    return interned_lexeme == interned_str;  // Comparación de punteros O(1)
}
```

## Tabla de Categorización Final

| Símbolo | Categoría | TokenType | Búsqueda Correcta |
|---------|-----------|-----------|-------------------|
| `=`, `+=`, `-=` | Asignación | `TOK_ASSIGNMENT_OP` | `is_assignment_intern()` |
| `.` | Acceso | `TOK_ARITHMETIC_OP` | `is_arithmetic_intern()` |
| `+`, `-`, `*`, `/`, `%`, `**` | Aritmético | `TOK_ARITHMETIC_OP` | `is_arithmetic_intern()` |
| `==`, `!=`, `>`, `<`, `>=`, `<=`, `&&`, `||`, `!` | Lógico | `TOK_LOGIC_OP` | `is_logic_intern()` |
| `;`, `:`, `{`, `}`, `,`, `(`, `)`, `[`, `]` | Delimitador | `TOK_DELIMITER` | `is_delim_intern()` |

## Migración a Interner

### Flujo de Trabajo
1. **Lexer**: Internalize todos los lexemas con `interner_intern_slice()`
2. **Parser**: Use comparaciones de punteros con símbolos precargados
3. **AST**: Almacene solo punteros internados, no copias de strings

### Ejemplo de Uso
```c
// Configuración inicial
Interner interner;
interner_init(&interner, arena, 256);
Interned syms;
interner_bootstrap_quimera(&interner, &syms);

// En el parser
if (is_kw_intern(p, at(p), p->syms->K_If)) {
    // Procesamiento de 'If' con comparación O(1)
}
```

## Beneficios de la Integración

### Rendimiento
- **Comparaciones O(1)** en lugar de O(n) para strings
- **Menor uso de memoria** por deduplicación automática
- **Cache-friendly** por localidad de punteros

### Mantenibilidad
- **Categorización consistente** entre lexer y parser
- **API unificada** para verificación de tokens
- **Menos duplicación de código**

### Robustez
- **Detección temprana** de problemas de categorización
- **Tests integrados** para verificar consistencia
- **Documentación clara** de responsabilidades

## Testing

El sistema incluye tests automatizados que verifican:
- Categorización correcta de todos los tokens
- Funcionamiento del interner
- Integración completa lexer-parser
- Análisis de programas de ejemplo

```c
// Test de integración exitoso
Integer x = 42;
Float y = 3.14;
String name = "Hello";
Boolean flag = True;

// Resultado: 4 declaraciones parseadas, 0 errores
```