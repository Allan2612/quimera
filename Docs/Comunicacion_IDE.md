# Comunicación IDE-Compilador de Quimera

## Resumen
Este documento describe el nuevo sistema de comunicación en tiempo real entre el IDE de Quimera y el compilador, implementado para permitir una integración fluida y retroalimentación inmediata de errores, warnings y salida de programas.

## Arquitectura del Sistema

### Principios de Diseño
1. **Comunicación sin buffer**: `stdout` y `stderr` configurados sin buffering para respuesta inmediata
2. **Formato JSON estructurado**: Toda la comunicación usa JSON para facilitar el procesamiento en Python
3. **Separación de canales**: 
   - `stderr`: errores de compilación, warnings y mensajes de diagnóstico
   - `stdout`: salida de programas interpretados
4. **Sin threads internos**: eliminación de complejidad innecesaria
5. **Archivos temporales**: el IDE escribe código en `tmp/ide_input.qmr` que el compilador lee

### Flujo de Trabajo
1. El IDE escribe código del usuario en `tmp/ide_input.qmr`
2. El IDE ejecuta `Quimera.exe --interactive` en un proceso separado
3. El compilador lee el archivo temporal y procesa el código
4. Los errores/warnings se envían por `stderr` en formato JSON
5. La salida del programa se envía por `stdout` en formato JSON
6. El IDE captura ambos canales y presenta la información al usuario

## Formatos de Mensaje

### Mensajes de Diagnóstico (stderr)

#### Estructura Base
```json
{
    "type": "diagnostic",
    "stage": "lexer|parser|semantic|runtime|system",
    "severity": "error|warning|info",
    "code": 1001,
    "line": 1,
    "column": 5,
    "length": 1,
    "lexeme": "@",
    "message": "Carácter no válido: '@'"
}
```

#### Códigos de Error por Etapa

**Errores del Lexer (1000-1999)**
- `1001`: Carácter no válido
- `1002`: Cadena sin cerrar
- `1003`: Comentario de bloque sin cerrar
- `1004`: Formato de número inválido
- `1005`: Literal de carácter inválido
- `1006`: Literal de carácter sin cerrar
- `1007`: Literal de carácter vacío
- `1008`: Secuencia de escape inválida

**Errores del Parser (2000-2999)**
- `2001`: Se esperaba ';'
- `2002`: Token inesperado
- `2003`: Se esperaba ')'
- `2004`: Se esperaba '}'
- `2005`: Se esperaba ']'
- `2006`: Se esperaba identificador
- `2007`: Se esperaba expresión
- `2008`: Se esperaba tipo
- `2009`: Se esperaba palabra clave específica
- `2010`: Se esperaba delimitador específico

**Errores Semánticos (3000-3999)**
- `3001`: Identificador no declarado
- `3002`: Variable ya declarada
- `3003`: Tipos incompatibles
- `3004`: No se puede asignar a esta expresión
- `3005`: Función no definida
- `3006`: Número incorrecto de argumentos

**Errores de Runtime (4000-4999)**
- `4001`: División por cero
- `4002`: Índice fuera de rango
- `4003`: Operación no válida entre tipos
- `4004`: Stack overflow
- `4005`: Valor nulo accedido
- `4006`: No hay más datos de entrada (`Read` sin entrada disponible)

**Errores del Sistema (5000-5999)**
- `5001`: Error al abrir archivo
- `5002`: Error de memoria
- `5003`: Archivo no encontrado
- `5004`: Error de E/O

### Mensajes de Salida del Programa (stdout)

#### Salida Normal de Print
```json
{
    "type": "output",
    "content": "Hello World",
    "output_type": "text"
}
```

#### Valores Tipados
```json
{
    "type": "output",
    "content": "42",
    "output_type": "integer"
}
```

```json
{
    "type": "output",
    "content": "3.14",
    "output_type": "float"
}
```

```json
{
    "type": "output",
    "content": "true",
    "output_type": "boolean"
}
```

#### Errores de Runtime del Programa
```json
{
    "type": "runtime_error",
    "message": "División por cero"
}
```

## Modos de Ejecución

### Modo Interactivo (`--interactive`)
- El compilador busca y lee `tmp/ide_input.qmr`
- Diseñado para uso con el IDE
- Configuración automática sin buffering

### Modo Normal
- `Quimera.exe archivo.qmr`
- Para uso desde línea de comandos
- Mantiene compatibilidad con scripts

## Implementación en el IDE (Python)

### Ejemplo de Captura de Salida
```python
import subprocess
import json
import threading
from queue import Queue

def run_compiler(code, callback):
    # Escribir código al archivo temporal
    with open('tmp/ide_input.qmr', 'w', encoding='utf-8') as f:
        f.write(code)
    
    # Ejecutar compilador
    process = subprocess.Popen(
        ['bin/Quimera.exe', '--interactive'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=0
    )
    
    # Threads para capturar salida en tiempo real
    def read_stderr():
        for line in process.stderr:
            try:
                msg = json.loads(line.strip())
                callback('diagnostic', msg)
            except json.JSONDecodeError:
                pass
    
    def read_stdout():
        for line in process.stdout:
            try:
                msg = json.loads(line.strip())
                callback('output', msg)
            except json.JSONDecodeError:
                pass
    
    threading.Thread(target=read_stderr, daemon=True).start()
    threading.Thread(target=read_stdout, daemon=True).start()
    
    return process
```

### Procesamiento de Mensajes
```python
def handle_message(msg_type, msg):
    if msg_type == 'diagnostic':
        if msg['severity'] == 'error':
            show_error(msg['line'], msg['column'], msg['message'])
        elif msg['severity'] == 'warning':
            show_warning(msg['line'], msg['column'], msg['message'])
    
    elif msg_type == 'output':
        if msg['type'] == 'output':
            append_output(msg['content'])
        elif msg['type'] == 'runtime_error':
            show_runtime_error(msg['message'])
```

## Ventajas del Nuevo Sistema

1. **Tiempo Real**: Sin buffering permite feedback inmediato
2. **Estructurado**: JSON facilita el procesamiento automático
3. **Escalable**: Fácil agregar nuevos tipos de mensaje
4. **Robusto**: Separación clara entre errores y salida
5. **Multiplataforma**: Funciona en Windows, Linux, macOS
6. **Debuggable**: Mensajes en formato legible para desarrollo

## Pruebas

### Verificar Funcionamiento Básico
```bash
# Crear archivo de prueba
echo 'Integer x = 42; Print(x);' > tmp/ide_input.qmr

# Ejecutar en modo interactivo
./bin/Quimera.exe --interactive
```

**Salida esperada:**
```json
{"type":"output","content":"42","output_type":"text"}
```

### Verificar Manejo de Errores
```bash
# Crear archivo con error de sintaxis
echo 'Inte ger x = 42;' > tmp/ide_input.qmr

# Ejecutar en modo interactivo
./bin/Quimera.exe --interactive
```

**Salida esperada (stderr):**
```json
{"type":"diagnostic","stage":"lexer","severity":"error","code":1001,"line":1,"column":5,"length":1,"lexeme":"g","message":"Carácter no válido: 'g'"}
```

## Consideraciones de Rendimiento

- El sistema es lo suficientemente rápido para edición en tiempo real
- Los archivos temporales son pequeños (código fuente únicamente)
- El procesamiento JSON es eficiente en Python moderno
- Se puede implementar debouncing en el IDE para evitar compilaciones excesivas

## Futuras Mejoras

1. **Highlighting**: Los campos `line`, `column`, `length` permiten resaltar errores exactos
2. **Completado**: Base para implementar autocompletado basado en análisis semántico
3. **Refactoring**: Información estructurada facilita operaciones de refactoring
4. **Métricas**: Posible recolección de métricas de uso y rendimiento
5. **Plugins**: API estructurada para extensiones del IDE