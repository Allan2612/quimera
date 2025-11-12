# Sistema de Diagnósticos Unificado - Quimera

## Resumen de la Migración

Este documento describe la migración completa del sistema de diagnósticos del compilador Quimera de un sistema basado en hilos a un sistema unificado JSON para integración con IDE.

## Arquitectura Nueva

### Archivo Principal: `diag.h` / `diag.c`

El nuevo sistema centraliza todo el manejo de errores, advertencias y salidas del programa en un solo lugar.

#### Características Principales:

1. **Sin buffering**: Output en tiempo real para IDE
2. **Formato JSON**: Salidas estructuradas para fácil parsing
3. **Códigos de error estructurados**: Por etapa del compilador
4. **Canales separados**: `stderr` para errores, `stdout` para salida de programa

### Códigos de Error por Etapa

```c
// Rangos de códigos de error:
#define ERROR_LEXER_BASE     1000  // 1000-1999
#define ERROR_PARSER_BASE    2000  // 2000-2999  
#define ERROR_SEMANTIC_BASE  3000  // 3000-3999
#define ERROR_RUNTIME_BASE   4000  // 4000-4999
#define ERROR_SYSTEM_BASE    5000  // 5000-5999
```

### API Principal

```c
// Inicialización del sistema
void diag_init(bool interactive_mode, const char *output_file);

// Reportar errores por etapa
void diag_error_lexer(int code, DiagLocation loc, const char *fmt, ...);
void diag_error_parser(int code, DiagLocation loc, const char *fmt, ...);
void diag_error_semantic(int code, DiagLocation loc, const char *fmt, ...);
void diag_error_runtime(int code, DiagLocation loc, const char *fmt, ...);
void diag_error_system(int code, DiagLocation loc, const char *fmt, ...);

// Salida de programa
void output_program_value(const char *value, const char *type);
void output_program_error(const char *error_msg);

// Limpieza
void diag_shutdown(void);
```

## Formato de Salida JSON

### Errores y Advertencias (stderr)
```json
{
    "severity": "error",
    "stage": "parser", 
    "code": 2001,
    "message": "se esperaba ';' al final de la declaración",
    "location": {
        "line": 15,
        "column": 23,
        "line_end": 15,
        "column_end": 23
    }
}
```

### Salida de Programa (stdout)
```json
{
    "type": "program_output",
    "data": {
        "value": "42",
        "value_type": "integer"
    }
}
```

### Errores de Ejecución (stdout)
```json
{
    "type": "program_error", 
    "data": {
        "error": "División por cero en línea 10"
    }
}
```

## Modo Interactivo para IDE

### Activación
```bash
./Quimera --interactive archivo.qmr
```

### Características:
- Sin buffering de salida
- Comunicación en tiempo real
- Archivo temporal para input del IDE: `tmp/ide_input.qmr`
- JSON estructurado para parsing automático

## Migración Completada

### Componentes Actualizados:

1. **Sistema de Diagnósticos**
   - ✅ `diag.h` / `diag.c` - Sistema nuevo unificado
   - ✅ Eliminado `diag_errors.h` - Sistema viejo obsoleto

2. **Compilador Principal**
   - ✅ `main.c` - Integrado modo interactivo
   - ✅ `lexer.c` - Migrado a nuevo sistema
   - ✅ `parser.c` - Limpiado y unificado con interner
   - ✅ `interpreter.c` - Salida JSON para resultados

3. **Tests**
   - ✅ Todos los archivos de test migrados a `diag.h`
   - ✅ 9 archivos actualizados correctamente

4. **Sistema de Build**
   - ✅ `Makefile` - Compatible con MSYS2 UCRT64
   - ✅ `build.bat` - Script personalizado para Windows
   - ✅ Compilación exitosa en ambos sistemas

## Parser Unificado

### Cambios Principales:
- ✅ Eliminadas funciones `lexeme_is` temporales
- ✅ Solo uso de `interner` para comparaciones
- ✅ Funciones `is_kw()`, `is_delim()`, `is_type()` unificadas
- ✅ Removed mixed implementation approach

### Funciones de Ayuda:
```c
// Función principal de comparación
static int lexeme_is_intern(Parser *p, const Token *t, const char *interned_str);

// Wrapper para strings literales  
static int lexeme_is(Parser *p, const Token *t, const char *s);

// Funciones de verificación unificadas
static int is_kw(Parser *p, const Token *t, const char *kw);
static int is_delim(Parser *p, const Token *t, const char *d);
static int is_type(Parser *p, const Token *t);
```

## Verificación Final

### Compilación
```bash
# Con Makefile
make debug

# Con script personalizado  
./build.bat
```

### Tests
```bash
# Ejecutar tests
make tests
```

### Modo Interactivo
```bash
# Probar IDE integration
./bin/Quimera.exe --interactive ejemplo.qmr
```

## Beneficios del Nuevo Sistema

1. **Comunicación en Tiempo Real**: Sin threads, sin buffering
2. **JSON Estructurado**: Fácil parsing para IDEs
3. **Códigos Organizados**: Por etapa para mejor debugging
4. **Modo Interactivo**: Diseñado específicamente para IDEs
5. **Sistema Unificado**: Una sola API para todos los diagnósticos
6. **Mejor Rendimiento**: Sin overhead de threads
7. **Compatibilidad**: Funciona en Windows con MSYS2

## Estado: COMPLETADO ✅

La migración ha sido completada exitosamente. El compilador Quimera ahora tiene:
- Sistema de diagnósticos unificado
- Integración completa con IDEs
- Compilación exitosa en todos los componentes
- Tests actualizados y funcionando
- Documentación completa

El puente efectivo entre el compilador y el IDE está listo para uso.