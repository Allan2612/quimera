# Intérprete Tree-Walker para Quimera

## Resumen

Se ha implementado exitosamente un intérprete tree-walker para el lenguaje Quimera que recorre y evalúa directamente el AST producido por el parser para ejecutar programas.

## Características Implementadas

### 1. Evaluación de Expresiones

- **Operaciones aritméticas**: `+`, `-`, `*`, `/`, `%`
- **Operaciones lógicas**: `And`, `Or`, `Not` con evaluación short-circuit
- **Operaciones de comparación**: `==`, `!=`, `<`, `<=`, `>`, `>=`
- **Operaciones unarias**: `-` (negación) y `Not`
- **Operaciones de asignación**: `=`, `+=`, `-=`

### 2. Sistema de Tipos

- **Tipos primitivos soportados**:
  - `Integer`: Números enteros
  - `Float`: Números de punto flotante
  - `String`: Cadenas de texto
  - `Boolean`: Valores lógicos (`True`/`False`)
  - `Char`: Caracteres individuales
  - `Null`: Ausencia de valor

### 3. Promoción de Tipos

- **Promoción automática** de `Integer → Float` en operaciones mixtas
- **Conversión automática** de `Char → String` en concatenaciones
- **Manejo robusto** de operaciones entre tipos incompatibles

### 4. Reglas de Truthiness

- `Null`: siempre falso
- `Boolean`: según su valor (`True`/`False`)
- `Integer`: falso si es `0`, verdadero en caso contrario
- `Float`: falso si es `0.0`, verdadero en caso contrario
- `String`: falso si está vacío (`""`), verdadero en caso contrario
- `Char`: falso si es `'\0'`, verdadero en caso contrario

### 5. Control de Flujo

- **Propagación sin goto**: El control de flujo se maneja mediante estados de retorno que se propagan en la pila de llamadas
- **Estados soportados**: `FLOW_NORMAL`, `FLOW_RETURN`, `FLOW_BREAK`, `FLOW_CONTINUE`, `FLOW_ERROR`

### 6. Gestión de Variables

- **Scoping**: Soporte completo para ámbitos anidados
- **Inicialización**: Variables sin inicializador explícito se inicializan a `Null`
- **Búsqueda**: Resolución de variables desde el scope actual hacia arriba en la cadena de scopes

### 7. Operaciones Especiales

- **Short-circuit evaluation**: 
  - `A And B`: no evalúa `B` si `A` es falso
  - `A Or B`: no evalúa `B` si `A` es verdadero
- **División por cero**: Emite warning y devuelve `0`
- **Concatenación de strings**: Manejo automático de comillas en strings literales

### 8. Sistema de Diagnósticos

- **Integración completa** con el sistema de errores existente
- **Errores semánticos**: Variables no declaradas, tipos incompatibles
- **Warnings de runtime**: División por cero, comparaciones entre tipos incompatibles

### 9. Funciones
- Declaración `Tipo nombre(Tipo param, ...) ... End` (o `Null` si no devuelve nada) y llamada `nombre(args)`.
- Cada llamada abre un scope cuyo padre es el global: la función ve las variables globales y sus parámetros, no las locales de quien la llama.
- `Return` termina la función; sin `Return` devuelve `Null`.
- Recursión con un límite de 400 llamadas anidadas (error 4004), para que una recursión sin caso base no tumbe el intérprete.

### 10. Arreglos y listas
- Son objetos por referencia: `l.append(x)` modifica la variable y pasarlos a una función no los copia.
- Arreglo: `Integer a[3];` o `Integer a[3] {1, 2, 3};`. Tamaño fijo; los elementos sin valor toman el valor por defecto de su tipo (0, 0.0, False, "").
- Lista: `Integer List l;` o `Integer List l {4, 5};`, con `l.append(x)`, `l.remove(posición)` y `l.size()`.
- Acceso y asignación por índice: `a[i]`, `a[i] = x`, `a[i] += x`. Un índice fuera de rango corta el programa (error 4002).
- `Print` muestra arreglos y listas como `[1, 2, 3]`.

### 11. Entrada con Read
- `Read(x);` lee una línea de la entrada estándar y la convierte al tipo declarado de `x` (Integer, Float, Boolean, Char o String).
- Si la entrada no corresponde al tipo, o no hay más entrada (error 4006), el programa termina con error.

### 12. Otros
- `Print` acepta varios valores separados por coma: `Print("x =", x);` imprime `x = 10`.
- Literales `Char` (`'A'`, `'\n'`) y concatenación de `Char` con `String`.
- La comparación de textos compara el contenido, venga de un literal, de una concatenación o de `Read`.
- La división o el módulo por cero cortan el programa (error 4001); si el divisor es un literal 0, lo detecta el análisis semántico.
- El ejecutable devuelve 0 si el programa terminó bien y 1 si hubo errores.

## Arquitectura del Intérprete

### Componentes Principales

1. **InterpreterContext**: Contexto global del intérprete
2. **Scope**: Sistema de manejo de ámbitos y variables
3. **Value**: Sistema de valores en tiempo de ejecución
4. **EvalResult**: Resultados de evaluación con control de flujo

### Flujo de Ejecución

```
Programa Fuente → Lexer → Parser → AST → Intérprete → Resultados/Efectos
```

## Archivos del Intérprete

- **`include/interpreter.h`**: Definiciones y API del intérprete
- **`src/interpreter/interpreter.c`**: Implementación completa del intérprete
- **`tests/interpreter/test_main.c`**: Suite de pruebas unitarias

## Ejemplos de Uso

### Ejemplo Básico

```quimera
Integer x = 10;
Integer y = 5;
Integer suma = x + y;
Print("La suma es:", suma);
```

### Promoción de Tipos

```quimera
Integer entero = 10;
Float decimal = 3.14;
Float resultado = entero + decimal; // Integer promovido a Float
Print("Resultado:", resultado); // Imprime: 15.14
```

### Operaciones Lógicas

```quimera
Boolean a = True;
Boolean b = False;
Boolean y_logico = a And b; // False
Boolean o_logico = a Or b;  // True
Boolean negacion = Not a;   // False
```

### Concatenación de Strings

```quimera
String saludo = "Hola" + " " + "Mundo";
Print(saludo); // Imprime: Hola Mundo
```

## Pruebas Unitarias

El intérprete incluye una suite completa de 8 pruebas unitarias que cubren:

1. **Operaciones aritméticas**: Precedencia y evaluación correcta
2. **Operaciones booleanas**: `And`, `Or`, `Not` con short-circuit
3. **Promoción de tipos**: `Integer → Float`
4. **Operaciones de string**: Concatenación
5. **Comparaciones**: Todos los operadores relacionales
6. **Asignaciones compuestas**: `+=`, `-=`
7. **Valores null**: Manejo de variables sin inicializar
8. **Truthiness**: Reglas de verdad para todos los tipos

## Estado Actual

✅ **Completamente funcional** para:
- Expresiones aritméticas y lógicas
- Declaraciones de variables
- Asignaciones simples y compuestas
- Sistema de tipos con promoción automática
- Print statements
- Manejo robusto de errores

🚧 **Pendiente de implementar**:
- Estructuras de control (`If/Else`, `While`, `For`)
- Declaración y llamada de funciones
- Listas y operaciones sobre arrays
- Read statements
- Return statements en funciones

## Compilación y Ejecución

### Compilar el intérprete:
```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -g -O0 -I include -c src/interpreter/interpreter.c -o obj/src/interpreter/interpreter.o
```

### Ejecutar pruebas:
```bash
./bin/tests/interpreter/test_main
```

### Ejecutar programa ejemplo:
```bash
./bin/Quimera
```

## Conclusión

El intérprete tree-walker proporciona una base sólida y funcional para la ejecución de programas Quimera. Su arquitectura modular permite extensiones futuras y su integración con el sistema de diagnósticos existente proporciona una experiencia de desarrollo robusta.