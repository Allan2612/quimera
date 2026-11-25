# Arquitectura y Flujo del Compilador Quimera - Documentación Completa

> **Visión General**: Arquitectura completa, integraciones y flujo del compilador Quimera  
> **Estado**: ✅ **SISTEMA COMPLETAMENTE INTEGRADO**

---

## 📋 Índice

1. [Arquitectura General](#arquitectura-general)
2. [Pipeline de Compilación](#pipeline-de-compilación)
3. [Flujo de Datos](#flujo-de-datos)
4. [Integraciones entre Componentes](#integraciones-entre-componentes)
5. [Gestión de Errores Global](#gestión-de-errores-global)
6. [Sistema de Memoria](#sistema-de-memoria)
7. [Rendimiento y Optimizaciones](#rendimiento-y-optimizaciones)
8. [Extensibilidad](#extensibilidad)
9. [Casos de Uso Completos](#casos-de-uso-completos)

---

## 🏗️ Arquitectura General

### Visión de Alto Nivel

```mermaid
graph TB
    subgraph "Entrada"
        A[Código Fuente .qmr]
    end
    
    subgraph "Frontend"
        B[Lexer<br/>Tokenización]
        C[Parser<br/>AST]
        D[Analyzer<br/>Semántica]
    end
    
    subgraph "Backend"
        E[Interpreter<br/>Ejecución]
        F[CodeGen<br/>Futuro]
    end
    
    subgraph "Soporte"
        G[Interner<br/>Strings]
        H[Diag<br/>Errores]
        I[AST Arena<br/>Memoria]
        J[Utils<br/>Comunes]
    end
    
    subgraph "Salida"
        K[Resultado<br/>Ejecución]
        L[Diagnósticos<br/>JSON]
    end
    
    A --> B
    B --> C
    C --> D
    D --> E
    E --> K
    E --> F
    
    G --> B
    G --> C
    G --> D
    G --> E
    
    H --> B
    H --> C
    H --> D
    H --> E
    H --> L
    
    I --> C
    I --> D
    I --> E
    
    J --> B
    J --> C
    J --> D
    J --> E
```

### Principios Arquitectónicos

#### 1. **Separación de Responsabilidades**
```c
// Cada fase tiene responsabilidad única y bien definida
Lexer:    source_code → tokens
Parser:   tokens → AST
Analyzer: AST → validated_AST
Interpreter: validated_AST → execution_result
```

#### 2. **Pipeline Secuencial con Recuperación**
```c
// Flujo principal que continúa incluso con errores
bool compile_and_run(const char *source) {
    // 1. Tokenización (continúa con errores)
    TokenList tokens = lexer_tokenize(source);
    
    // 2. Análisis sintáctico (con recovery)
    AstNode *ast = parser_parse(tokens);
    
    // 3. Análisis semántico (procesa AST dañado)
    bool semantic_ok = analyzer_analyze(ast);
    
    // 4. Ejecución (solo si no hay errores)
    if (semantic_ok && no_critical_errors) {
        interpreter_execute(ast);
        return true;
    }
    
    return false; // Solo reportar errores, no ejecutar
}
```

#### 3. **Recursos Compartidos**
```c
// Todas las fases comparten recursos base
typedef struct {
    AstArena *arena;        // Memoria unificada
    Interner *interner;     // Strings compartidos
    Interned *symbols;      // Símbolos del lenguaje
} CompilerContext;
```

#### 4. **Error Handling Unificado**
```c
// Sistema de diagnósticos centralizado
DiagSystem → JSON estructurado → IDE/Usuario
```

---

## 🔄 Pipeline de Compilación

### Fases del Pipeline

#### **Fase 1: Análisis Léxico**
```c
Input:  Código fuente como string
Process: Tokenización con identificación de caracteres inválidos
Output: Lista de tokens (incluyendo TOK_ERROR)
Errors: Errores léxicos (código 1xxx)

// Características especiales:
- Continúa tras caracteres inválidos
- Genera TOK_ERROR para caracteres no reconocidos
- Optimizado con string interning
```

#### **Fase 2: Análisis Sintáctico**
```c
Input:  Lista de tokens del lexer
Process: Parsing descendente recursivo con recovery
Output: AST (incluyendo nodos AST_ERROR)
Errors: Errores sintácticos (código 2xxx)

// Características especiales:
- Panic mode recovery en puntos de sincronización
- Maneja TOK_ERROR específicamente (código 2003)
- Genera AST parcial para análisis posterior
```

#### **Fase 3: Análisis Semántico**
```c
Input:  AST del parser (puede contener errores)
Process: Validación de tipos, scope, semántica
Output: AST validado o rechazado
Errors: Errores semánticos (código 3xxx)

// Características especiales:
- Procesa AST con nodos de error sintáctico
- Detección múltiple de errores en una pasada
- Scope management con hash tables eficientes
```

#### **Fase 4: Interpretación**
```c
Input:  AST validado semánticamente
Process: Ejecución walking AST
Output: Resultados del programa
Errors: Errores runtime (código 4xxx)

// Características especiales:
- Solo ejecuta código completamente validado
- Walking interpreter directo del AST
- Gestión dinámica de variables y scope
```

### Puntos de Control

```c
// Cada fase tiene puntos de control específicos
typedef struct {
    bool should_continue;    // ¿Continuar a siguiente fase?
    bool has_critical_errors; // ¿Errores que impiden ejecución?
    int error_count;        // Número de errores encontrados
    int warning_count;      // Número de warnings
} PhaseStatus;

// Control del pipeline
PhaseStatus lexer_status = lexer_run(source);
if (!lexer_status.should_continue) return COMPILE_FAILED;

PhaseStatus parser_status = parser_run(tokens);
if (!parser_status.should_continue) return COMPILE_FAILED;

PhaseStatus semantic_status = analyzer_run(ast);
if (!semantic_status.should_continue) return COMPILE_FAILED;

// Solo ejecutar si no hay errores críticos
if (!semantic_status.has_critical_errors) {
    interpreter_run(validated_ast);
    return COMPILE_SUCCESS;
}
```

---

## 📊 Flujo de Datos

### Estructuras de Datos Principales

#### **1. Token Flow (Lexer → Parser)**
```c
Token {
    TokenType type;        // Categoría del token
    SrcSpan span;         // Posición en código fuente
    const char *lexeme;   // Texto internado
}

TokenList {
    Token *tokens;        // Array dinámico
    size_t count;         // Número de tokens
    size_t capacity;      // Capacidad actual
}
```

#### **2. AST Flow (Parser → Analyzer → Interpreter)**
```c
AstNode {
    AstNodeType type;     // Tipo del nodo
    SrcSpan span;         // Información de posición
    union data;           // Datos específicos por tipo
}

// Nodos especiales para manejo de errores
AST_ERROR {
    const char *message;          // Descripción del error
    TokenType error_token_type;   // Token que causó problema
}
```

#### **3. Symbol Flow (Analyzer)**
```c
Symbol {
    const char *name;     // Nombre internado
    TypeInfo type;        // Información de tipo
    SrcSpan declared_at;  // Ubicación de declaración
    bool is_initialized;  // Estado de inicialización
}

AnalyzerScope {
    Symbol **symbols;     // Hash table de símbolos
    size_t capacity;      // Capacidad de la tabla
    AnalyzerScope *parent; // Scope padre
}
```

#### **4. Value Flow (Interpreter)**
```c
Value {
    ValueType type;       // Tipo runtime
    union as;             // Valor específico por tipo
}

EvalResult {
    Value value;          // Resultado de evaluación
    FlowControl flow;     // Control de flujo (normal/return/break/etc.)
}
```

### Transformaciones de Datos

```mermaid
graph LR
    A[Source Code] --> B[Tokens]
    B --> C[Raw AST]
    C --> D[Validated AST]
    D --> E[Runtime Values]
    
    subgraph "Metadatos"
        F[SrcSpan Info]
        G[Type Info]
        H[Symbol Table]
        I[Diagnostics]
    end
    
    F --> B
    F --> C
    G --> D
    H --> D
    I --> B
    I --> C
    I --> D
    I --> E
```

---

## 🔗 Integraciones entre Componentes

### **Integración Lexer ↔ Parser**

#### Interfaz de Comunicación
```c
// Parser consume tokens del lexer
typedef struct {
    const Token *toks;    // Array de tokens
    size_t count;         // Número total
    size_t i;            // Índice actual
} ParserInput;

// Funciones de navegación
static inline const Token *at(Parser *p);
static inline const Token *advance(Parser *p);
static inline const Token *peek(Parser *p, size_t k);
```

#### Manejo de TOK_ERROR
```c
// Parser maneja específicamente tokens de error del lexer
if (tok_is(current_token, TOK_ERROR)) {
    // Generar diagnóstico específico
    diag_error(STAGE_PARSER, 2003, token_location, 
              "Token de error léxico en análisis sintáctico");
    
    // Aplicar recovery y continuar
    panic_mode_recovery(parser);
    continue;
}
```

### **Integración Parser ↔ Analyzer**

#### Manejo de AST Dañado
```c
// Analyzer procesa AST incluso con nodos de error
bool analyze_statement(SemanticAnalyzer *analyzer, AstNode *stmt) {
    // Manejar nodos de error del parser
    if (stmt->type == AST_ERROR) {
        analyzer->error_count++;
        // Continuar con siguiente nodo, no abortar
        return false;
    }
    
    // Análisis normal de nodos válidos
    return analyze_statement_normal(analyzer, stmt);
}
```

#### Preservación de Información
```c
// El analyzer preserva información de posición del parser
void semantic_error(SemanticAnalyzer *analyzer, SrcSpan span, 
                   int code, const char *message) {
    DiagLocation loc = diag_location_create(
        span.line_start, span.col_start, 
        span.col_end - span.col_start, ""
    );
    diag_error(STAGE_SEMANTIC, code, loc, message);
    analyzer->error_count++;
}
```

### **Integración Analyzer ↔ Interpreter**

#### Validación Previa
```c
// Interpreter solo ejecuta código validado
bool interpret_program(InterpreterContext *ctx, AstNode *program) {
    // Verificación previa: el AST debe estar validado
    if (ctx->has_semantic_errors) {
        runtime_error(ctx, program->span, 
                     "No se puede ejecutar código con errores semánticos");
        return false;
    }
    
    // Ejecución normal
    return execute_statements(ctx, program);
}
```

#### Confianza en Tipos
```c
// Interpreter confía en validación semántica
Value eval_binary_operation(InterpreterContext *ctx, AstNode *op) {
    // No necesita verificar compatibilidad de tipos
    // El analyzer ya validó que la operación es semánticamente correcta
    Value left = eval_expression(ctx, op->left);
    Value right = eval_expression(ctx, op->right);
    
    return apply_operator(op->operator, left, right);
}
```

### **Integración con Sistema de Diagnósticos**

#### Reportes Coordinados
```c
// Cada fase contribuye al mismo sistema de diagnósticos
void compile_file(const char *filename) {
    diag_set_json_output(true);
    
    // Lexer errores
    TokenList tokens = lexer_run(source); // Genera códigos 1xxx
    
    // Parser errores  
    AstNode *ast = parser_run(tokens);    // Genera códigos 2xxx
    
    // Analyzer errores
    bool valid = analyzer_run(ast);       // Genera códigos 3xxx
    
    // Interpreter errores
    if (valid) {
        interpreter_run(ast);             // Genera códigos 4xxx
    }
    
    // Sistema automáticamente agrega metadatos JSON
}
```

#### Correlación de Errores
```c
// Diagnósticos incluyen información de fase y correlación
{
  "type": "diagnostic",
  "stage": "semantic",
  "severity": "error",
  "code": 3008,
  "line": 15,
  "column": 8,
  "length": 5,
  "lexeme": "undefined_var",
  "message": "Identificador no declarado: 'undefined_var'",
  "related": [
    {
      "stage": "parser",
      "line": 15,
      "message": "Identificador parseado correctamente pero no declarado"
    }
  ]
}
```

---

## 🛡️ Gestión de Errores Global

### **Estrategia de Error Handling**

#### Principios Fundamentales
```c
1. **No Abortar**: Continuar análisis para detectar múltiples errores
2. **Información Rica**: Ubicación precisa y mensajes descriptivos  
3. **Recuperación Inteligente**: Recovery en puntos de sincronización
4. **Categorización**: Códigos específicos por fase y tipo de error
```

#### Implementación por Fases

```c
// Lexer: Generar TOK_ERROR y continuar
if (invalid_character) {
    create_error_token(lexer, current_char);
    advance_position(lexer);
    continue; // No abortar tokenización
}

// Parser: Panic mode recovery
if (syntax_error) {
    create_ast_error_node(parser);
    panic_mode_recovery(parser);
    continue; // Continuar parsing
}

// Analyzer: Validar AST dañado
if (ast_node->type == AST_ERROR) {
    analyzer->error_count++;
    return false; // Continuar con siguiente nodo
}

// Interpreter: Solo ejecutar código válido
if (analyzer_error_count > 0) {
    return; // No ejecutar, solo reportar errores
}
```

### **Sistema de Códigos de Error**

```c
// Organización jerárquica de códigos
1000-1999: Errores léxicos
  1001: Carácter inválido
  1002: String no terminado
  1003: Carácter no terminado

2000-2999: Errores sintácticos  
  2002: Token inesperado
  2003: Token de error léxico
  2004: Punto y coma faltante
  2005: Llave faltante
  2006: Paréntesis faltante

3000-3999: Errores semánticos
  3001: Tipo desconocido
  3003: Tipos incompatibles
  3008: Identificador no declarado
  3025: Return con valor en función void

4000-4999: Errores runtime
  4001: División por cero
  4002: Índice fuera de rango
  4003: Variable no inicializada

5000-5999: Errores del sistema
  5001: Sin memoria
  5002: Archivo no encontrado
```

### **Recovery Inteligente**

#### Puntos de Sincronización
```c
// Parser recovery busca tokens de sincronización
void panic_mode_recovery(Parser *p) {
    while (!is_eof(p)) {
        const Token *t = at(p);
        
        // Puntos donde se puede continuar parsing
        if (is_delim(p, t, ";") ||      // Fin de sentencia
            is_delim(p, t, "}") ||      // Fin de bloque
            is_kw(p, t, "If") ||        // Inicio de condicional
            is_kw(p, t, "While") ||     // Inicio de bucle
            is_type(p, t)) {            // Inicio de declaración
            break;
        }
        
        advance(p); // Saltar token problemático
    }
}
```

#### Continuación Semántica
```c
// Analyzer continúa validando elementos válidos
bool analyze_program(SemanticAnalyzer *analyzer, AstNode *program) {
    bool overall_success = true;
    
    for (size_t i = 0; i < program->statement_count; i++) {
        AstNode *stmt = program->statements[i];
        
        if (stmt->type == AST_ERROR) {
            overall_success = false;
            continue; // Saltar pero no abortar
        }
        
        if (!analyze_statement(analyzer, stmt)) {
            overall_success = false;
            // Continuar con siguiente sentencia
        }
    }
    
    return overall_success;
}
```

---

## 🧠 Sistema de Memoria

### **Arena Allocation**

#### Ventajas del Modelo Arena
```c
1. **Asignación Rápida**: O(1) allocation (solo incremento de offset)
2. **Cache Friendly**: Datos relacionados están cerca en memoria
3. **Sin Fragmentación**: Un bloque contiguo grande
4. **Liberación Eficiente**: Toda la arena se libera en bloque
5. **Gestión Simplificada**: No tracking individual de allocations
```

#### Implementación
```c
typedef struct {
    char *memory;         // Bloque de memoria contiguo
    size_t size;          // Tamaño usado actualmente  
    size_t capacity;      // Capacidad total
    size_t alignment;     // Alineación para punteros
} AstArena;

void *ast_alloc(AstArena *arena, size_t size) {
    // Alinear tamaño
    size_t aligned_size = (size + arena->alignment - 1) & ~(arena->alignment - 1);
    
    // Verificar espacio disponible
    if (arena->size + aligned_size > arena->capacity) {
        arena_resize(arena, arena->capacity * 2);
    }
    
    // Asignar y actualizar offset
    void *ptr = arena->memory + arena->size;
    arena->size += aligned_size;
    
    return ptr;
}
```

### **String Interning para Eficiencia**

#### Beneficios Medibles
```c
// Sin interning:
bool is_keyword = (strcmp(token.lexeme, "Integer") == 0); // O(n)

// Con interning:
bool is_keyword = (token.lexeme == symbols.T_Integer);    // O(1)

// Ahorro de memoria:
// 1000 identificadores "x" → 1000 copias vs 1 copia internada
// String "Integer" usado 500 veces → 1 copia vs 500 copias
```

#### Estrategia de Uso
```c
// Todos los strings del compilador son internados
const char *intern_token_lexeme(Parser *p, const Token *t) {
    return interner_intern_cstr(p->interner, t->lexeme);
}

// Comparaciones eficientes en toda la codebase
if (identifier == symbols->K_If) { /* Es 'If' */ }
if (operator == symbols->O_Plus) { /* Es '+' */ }
```

### **Gestión de Recursos Coordinada**

#### Ciclo de Vida
```c
int main() {
    // 1. Inicialización ordenada
    AstArena arena;
    Interner interner;
    Interned symbols;
    
    ast_arena_init(&arena, INITIAL_ARENA_SIZE);
    interner_init(&interner);
    interner_init_symbols(&interner, &symbols);
    
    // 2. Uso durante compilación
    // (Todas las fases comparten mismos recursos)
    
    // 3. Liberación en orden reverso
    // (No necesario liberar AST nodes individuales)
    interner_destroy(&interner);
    ast_arena_destroy(&arena);
    
    return 0;
}
```

---

## ⚡ Rendimiento y Optimizaciones

### **Optimizaciones Implementadas**

#### 1. **String Interning**
```c
// Tiempo de búsqueda de keywords: O(n) → O(1)
// Memoria: Linear → Logarítmica para strings duplicados
// Cache performance: Mejor localidad espacial
```

#### 2. **Arena Allocation**  
```c
// Tiempo de allocation: malloc() → pointer increment
// Fragmentación: Eliminada completamente
// Liberación: Individual free() → bulk free
```

#### 3. **Hash Tables Eficientes**
```c
// Symbol lookup en analyzer: O(n) linear scan → O(1) hash
// Rehashing automático para mantener load factor < 0.75
// Resolución de colisiones con chaining
```

#### 4. **Single Pass Analysis**
```c
// Cada fase procesa el input una sola vez
// No re-parsing o re-analysis innecesarios
// Information forward-flow eficiente
```

### **Métricas de Rendimiento**

#### Complejidades Temporales
```c
Lexer:      O(n) donde n = caracteres en source
Parser:     O(n) donde n = número de tokens  
Analyzer:   O(n) donde n = número de nodos AST
Interpreter: O(n) donde n = sentencias ejecutadas

Overall:    O(n) lineal en tamaño del programa
```

#### Uso de Memoria
```c
Arena Size:     ~1MB inicial, crece según necesidad
Token Storage:  ~20 bytes per token promedio
AST Storage:    ~50 bytes per nodo promedio  
Symbol Tables:  ~100 bytes per símbolo promedio

Peak Memory:    ~2-5MB para programas típicos
```

### **Benchmarks Estimados**

```c
// Programa típico de 1000 líneas:
Lexing:     < 5ms
Parsing:    < 10ms  
Analysis:   < 15ms
Execution:  Variable (depende de algoritmo)

Total Compilation: < 30ms
Memory Usage:      < 5MB
```

---

## 🔧 Extensibilidad

### **Puntos de Extensión Diseñados**

#### 1. **Nuevos Tipos de Datos**
```c
// En analyzer.h
typedef enum {
    TYPE_INTEGER, TYPE_FLOAT, TYPE_STRING, TYPE_BOOLEAN, TYPE_CHAR,
    TYPE_NULL, TYPE_ARRAY, TYPE_LIST, TYPE_FUNCTION, TYPE_VOID,
    
    // Extensión futura:
    TYPE_STRUCT,     // Estructuras definidas por usuario
    TYPE_ENUM,       // Enumeraciones
    TYPE_POINTER,    // Punteros (si se añaden)
    TYPE_OBJECT      // Objetos/clases
} SemanticType;
```

#### 2. **Nuevas Sentencias**
```c
// En ast.h
typedef enum {
    AST_DECLARATION, AST_ASSIGNMENT, AST_IF, AST_WHILE, AST_FOR,
    AST_RETURN, AST_PRINT, AST_READ, AST_FUNCTION_DEF, AST_BLOCK,
    
    // Extensión futura:
    AST_TRY_CATCH,   // Manejo de excepciones
    AST_IMPORT,      // Sistema de módulos
    AST_CLASS_DEF,   // Definición de clases
    AST_MATCH        // Pattern matching
} AstNodeType;
```

#### 3. **Nuevos Backends**
```c
// En codegen.h - Base para múltiples targets
typedef enum {
    CODEGEN_C,           // Generación de código C
    CODEGEN_JAVASCRIPT,  // Transpilación a JavaScript
    CODEGEN_PYTHON,      // Transpilación a Python  
    CODEGEN_BYTECODE,    // Bytecode para VM
    CODEGEN_LLVM         // LLVM IR para compilación nativa
} CodegenTarget;

bool codegen_generate(CodeGenerator *gen, AstNode *program, CodegenTarget target);
```

#### 4. **Sistema de Plugins**
```c
// Arquitectura preparada para plugins
typedef struct {
    const char *name;
    const char *version;
    
    // Hooks en diferentes fases
    bool (*lexer_hook)(LexerContext *ctx);
    bool (*parser_hook)(ParserContext *ctx);
    bool (*analyzer_hook)(AnalyzerContext *ctx);
    bool (*interpreter_hook)(InterpreterContext *ctx);
    
    // Configuración
    void (*configure)(PluginConfig *config);
} CompilerPlugin;
```

### **API para Extensiones**

#### Interfaz de Visitor Pattern
```c
// Para recorrido y transformación de AST
typedef struct {
    void (*visit_declaration)(AstVisitor *visitor, AstNode *node);
    void (*visit_assignment)(AstVisitor *visitor, AstNode *node);
    void (*visit_binary_op)(AstVisitor *visitor, AstNode *node);
    // ... más visits por tipo de nodo
    
    void *context; // Datos específicos del visitor
} AstVisitor;

void ast_walk(AstNode *root, AstVisitor *visitor);
```

#### Sistema de Macros (Futuro)
```c
// Preparación para sistema de macros
typedef struct {
    const char *name;           // Nombre del macro
    AstNode **parameters;       // Parámetros formales
    AstNode *expansion;         // Cuerpo del macro
    MacroType type;            // Tipo de macro (expresión/sentencia/declaración)
} MacroDefinition;

// Expandir macros durante parsing
AstNode *expand_macro(Parser *p, const char *macro_name, AstNode **args);
```

---

## 💡 Casos de Uso Completos

### **Caso 1: Programa Simple Sin Errores**

#### Código Fuente
```quimera
Integer x = 10;
Integer y = x + 5;
Print y;
```

#### Flujo Completo
```c
// 1. LEXER
Tokens generados: [
  {TOK_RESERVED_WORD, "Integer", line:1, col:1},
  {TOK_IDENTIFIER, "x", line:1, col:9},
  {TOK_ASSIGNMENT_OP, "=", line:1, col:11},
  {TOK_INTEGER, "10", line:1, col:13},
  {TOK_DELIMITER, ";", line:1, col:15},
  {TOK_RESERVED_WORD, "Integer", line:2, col:1},
  {TOK_IDENTIFIER, "y", line:2, col:9},
  {TOK_ASSIGNMENT_OP, "=", line:2, col:11},
  {TOK_IDENTIFIER, "x", line:2, col:13},
  {TOK_ARITHMETIC_OP, "+", line:2, col:15},
  {TOK_INTEGER, "5", line:2, col:17},
  {TOK_DELIMITER, ";", line:2, col:18},
  {TOK_RESERVED_WORD, "Print", line:3, col:1},
  {TOK_IDENTIFIER, "y", line:3, col:7},
  {TOK_DELIMITER, ";", line:3, col:8}
]
Errores: 0

// 2. PARSER
AST generado: Program {
  statements: [
    Declaration { type: "Integer", name: "x", init: IntegerLiteral(10) },
    Declaration { type: "Integer", name: "y", 
                 init: BinaryOp("+", Identifier("x"), IntegerLiteral(5)) },
    Print { expr: Identifier("y") }
  ]
}
Errores: 0

// 3. ANALYZER
Scope analysis:
  - Global scope: {x: Integer, y: Integer}
  - x declarado línea 1, inicializado con 10
  - y declarado línea 2, inicializado con x + 5
  - Operación x + 5: Integer + Integer → Integer ✓
  - Print y: y es Integer ✓
Errores: 0

// 4. INTERPRETER
Ejecución:
  - Crear variable x = 10
  - Evaluar x + 5: lookup x (10) + 5 = 15
  - Crear variable y = 15
  - Evaluar Print y: lookup y (15)
  - Imprimir: 15

Resultado: Salida "15", compilación y ejecución exitosa
```

### **Caso 2: Programa con Errores Múltiples**

#### Código Fuente
```quimera
@invalid!           // Error léxico
Integer x = ;       // Error sintáctico  
y = x + 5;         // Error semántico (y no declarado)
Integer z = x + "text"; // Error semántico (tipos incompatibles)
```

#### Flujo con Recovery
```c
// 1. LEXER
Tokens: [
  {TOK_ERROR, "@", line:1, col:1},        // Error léxico
  {TOK_IDENTIFIER, "invalid", line:1, col:2},
  {TOK_ERROR, "!", line:1, col:9},        // Error léxico  
  {TOK_RESERVED_WORD, "Integer", line:2, col:1},
  {TOK_IDENTIFIER, "x", line:2, col:9},
  {TOK_ASSIGNMENT_OP, "=", line:2, col:11},
  {TOK_DELIMITER, ";", line:2, col:13},   // Punto y coma inmediato
  // ... continúa tokenizando
]
Errores léxicos: 2

// 2. PARSER
- Encuentra TOK_ERROR: reporta error 2003, aplica recovery
- "Integer x =" seguido de ";": reporta error 2007 (expresión faltante)
- Aplica panic recovery hasta siguiente declaración
- Continúa parsing resto del programa

AST: Program {
  statements: [
    ASTError { message: "Token de error léxico" },
    ASTError { message: "Expresión faltante en inicialización" },
    Assignment { var: "y", value: BinaryOp("+", Identifier("x"), IntegerLiteral(5)) },
    Declaration { type: "Integer", name: "z", 
                 init: BinaryOp("+", Identifier("x"), StringLiteral("text")) }
  ]
}
Errores sintácticos: 2 (más los 2 léxicos propagados)

// 3. ANALYZER
- Nodo AST_ERROR: incrementa error count, continúa
- Assignment a "y": variable no declarada → error 3008
- Declaration z: analiza x + "text"
  * x declarado: ✓ (asume Integer por contexto)
  * "text": String ✓  
  * Integer + String: incompatible → error 3005
- Continúa análisis de todo el programa

Errores semánticos: 2
Total errores: 6 (2 léxicos + 2 sintácticos + 2 semánticos)

// 4. INTERPRETER
No se ejecuta debido a errores. Solo se reportan todos los diagnósticos encontrados.
```

#### Diagnósticos JSON Generados
```json
[
  {
    "type": "diagnostic", "stage": "lexical", "severity": "error",
    "code": 1001, "line": 1, "column": 1, "length": 1, "lexeme": "@",
    "message": "Carácter inválido en código fuente: '@'"
  },
  {
    "type": "diagnostic", "stage": "lexical", "severity": "error", 
    "code": 1001, "line": 1, "column": 9, "length": 1, "lexeme": "!",
    "message": "Carácter inválido en código fuente: '!'"
  },
  {
    "type": "diagnostic", "stage": "parser", "severity": "error",
    "code": 2003, "line": 1, "column": 1, "length": 1, "lexeme": "@", 
    "message": "Token de error léxico encontrado durante análisis sintáctico"
  },
  {
    "type": "diagnostic", "stage": "parser", "severity": "error",
    "code": 2007, "line": 2, "column": 13, "length": 1, "lexeme": ";",
    "message": "Se esperaba expresión en inicialización de variable"
  },
  {
    "type": "diagnostic", "stage": "semantic", "severity": "error",
    "code": 3008, "line": 3, "column": 1, "length": 1, "lexeme": "y",
    "message": "Identificador no declarado: 'y'"
  },
  {
    "type": "diagnostic", "stage": "semantic", "severity": "error",
    "code": 3005, "line": 4, "column": 13, "length": 11, "lexeme": "x + \"text\"",
    "message": "Operación aritmética entre tipos incompatibles: Integer y String"
  }
]
```

### **Caso 3: Programa Complejo Exitoso**

#### Código Fuente
```quimera
Integer factorial(Integer n) {
    If n <= 1 {
        Return 1;
    } Else {
        Return n * factorial(n - 1);
    }
} End

Integer result = factorial(5);
Print result;
```

#### Análisis Detallado
```c
// LEXER: 45+ tokens generados correctamente
// PARSER: AST con función, condicional, recursión
// ANALYZER:
//   - función factorial: Integer → Integer ✓
//   - parámetro n: Integer ✓
//   - condición n <= 1: Integer <= Integer → Boolean ✓
//   - Return 1: Integer compatible con retorno Integer ✓
//   - Return n * factorial(n-1): Integer * Integer → Integer ✓
//   - Llamada recursiva: factorial(n-1) → Integer ✓
//   - result = factorial(5): Integer ✓
// INTERPRETER: 
//   - Ejecuta factorial(5) recursivamente
//   - Resultado: 120
```

---

## 🎯 Resumen Arquitectónico

### **Características del Sistema**

#### ✅ **Pipeline Robusto**
- **4 fases integradas**: Lexer → Parser → Analyzer → Interpreter
- **Recovery inteligente** en cada fase
- **Detección múltiple** de errores sin aborto
- **Flujo de datos eficiente** entre componentes

#### ✅ **Sistema de Soporte Completo**
- **String interning** para optimización
- **Arena allocation** para gestión eficiente de memoria
- **Diagnósticos unificados** con JSON estructurado
- **Hash tables eficientes** para símbolos y scopes

#### ✅ **Manejo de Errores Excepcional**
- **Códigos organizados** por fase (1xxx, 2xxx, 3xxx, 4xxx)
- **Información rica** de ubicación y contexto
- **Correlación entre fases** para errores relacionados
- **No abort policy**: máxima detección de problemas

#### ✅ **Rendimiento Optimizado**
- **Complejidad O(n)** lineal en tamaño de programa
- **Memoria eficiente** con arena y string interning
- **Single pass** por cada fase
- **Cache friendly** data structures

#### ✅ **Extensibilidad Preparada**
- **APIs bien definidas** para cada componente
- **Visitor pattern** para traversal de AST
- **Plugin architecture** preparada
- **Multiple backends** posibles (CodeGen)

### **🏆 Logros Arquitectónicos**

1. **Sistema Completo**: Compilador funcional end-to-end
2. **Robustez Excepcional**: Maneja cualquier input sin crashes
3. **Performance Eficiente**: Optimizado para velocidad y memoria
4. **Código Mantenible**: Arquitectura limpia y modular
5. **Extensible**: Preparado para crecimiento futuro
6. **IDE-Ready**: Diagnósticos JSON para integración con IDEs

---

**El compilador Quimera representa una implementación completa, robusta y eficiente de un sistema de compilación moderno, con arquitectura extensible y manejo excepcional de errores.**