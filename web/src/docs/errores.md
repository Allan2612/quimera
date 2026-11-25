---
description: Qué significa cada código de error de Quimera (léxico, sintáctico, semántico y de ejecución), con un ejemplo que lo provoca y cómo arreglarlo.
---
# Errores

Cada error dice la **etapa**, la **línea y columna**, un **código** y un mensaje. El código indica la etapa:

| Códigos | Etapa | Cuándo aparece |
|---|---|---|
| 1000–1999 | Léxico | al leer el texto, antes de entender el programa |
| 2000–2999 | Sintaxis | al armar la estructura del programa |
| 3000–3999 | Semántica | al revisar tipos y nombres, antes de ejecutar |
| 4000–4999 | Ejecución | mientras el programa corre |
| 5000–5999 | Sistema | al abrir archivos o reservar memoria |

**Arregla primero el primer error.** Uno léxico o de sintaxis suele arrastrar otros después (por ejemplo, "Token de error del lexer - recuperando análisis" o "Nodo AST no soportado") que desaparecen solos al corregir el primero.

Cada ejemplo de esta página provoca el error que explica: ábrelo en el editor y arréglalo.

## Errores léxicos

### 1001 · Carácter no válido

El texto tiene un símbolo que no es parte del lenguaje, como `@`, `$` o `#`.

```qmr error=1001
Integer x = 5 @ 3;
Print(x);
```

**Cómo arreglarlo:** quita el símbolo o cámbialo por un operador válido (`+ - * / %`). Para comentarios se usa `//`, no `#`.

### 1003 · Comentario de bloque sin cerrar

Se abrió un comentario con `/*` y nunca se cerró con `*/`, así que todo lo que sigue cuenta como comentario.

```qmr error=1003
/* Este comentario no termina
Print("No se va a ejecutar");
```

**Cómo arreglarlo:** cierra el comentario con `*/`.

### 1004 · Formato de número inválido

Un número con dos puntos decimales u otra forma que no se puede leer.

```qmr error=1004
Float precio = 3.4.5;
```

**Cómo arreglarlo:** deja un solo punto decimal: `3.45`.

### 1005 · Literal de carácter inválido

Un `Char` lleva un solo carácter entre comillas simples.

```qmr error=1005
Char letra = 'ab';
```

**Cómo arreglarlo:** usa un solo carácter (`'a'`) o cambia el tipo a `String` con comillas dobles (`"ab"`).

### 1006 · Literal sin cerrar

Falta la comilla de cierre. Aparece tanto en un `Char` (`'a`) como en un texto (`"hola`).

```qmr error=1006
String saludo = "hola;
Print(saludo);
```

**Cómo arreglarlo:** cierra el texto con la misma comilla con la que lo abriste: `"hola"`.

### 1007 · Literal de carácter vacío

Dos comillas simples seguidas, sin nada adentro.

```qmr error=1007
Char c = '';
```

**Cómo arreglarlo:** pon un carácter entre las comillas, o usa un `String` vacío (`""`).

## Errores de sintaxis

### 2001 · Bloque sin cerrar

Una advertencia que acompaña al [2009](#e2009): un `If`, `While`, `For` o función llegó al final del archivo sin su `End`.

### 2002 · Token inesperado

Algo aparece donde no corresponde, por ejemplo una asignación sin nombre de variable o una expresión que empieza con un operador.

```qmr error=2002
Integer = 5;
```

**Cómo arreglarlo:** revisa la línea marcada: cada declaración es `Tipo nombre = valor;`.

### 2009 · Se esperaba una palabra clave

Casi siempre es un `End` que falta.

```qmr error=2009
If (True)
  Print("falta el End");
```

**Cómo arreglarlo:** cierra cada `If`, `While`, `For` y función con `End`. Un `If` con `ElseIf` o `Else` lleva un solo `End`, al final.

### 2010 · Se esperaba un delimitador

Falta un `;`, un paréntesis o un corchete. El mensaje dice cuál.

```qmr error=2010
Integer x = 5
Print(x);
```

**Cómo arreglarlo:** agrega lo que indica el mensaje. Si falta un `;`, el error suele marcar el comienzo de la línea siguiente.

## Errores semánticos

### 3000 · Error semántico

El programa está bien escrito, pero no tiene sentido: un nombre que no existe, tipos que no combinan o una llamada con argumentos de más. Todos usan el código 3000; el mensaje dice cuál es. Los más comunes:

| Mensaje | Qué pasa |
|---|---|
| Identificador no declarado | se usa una variable o función que no existe, o que se declara más abajo |
| Variable ya declarada | el mismo nombre dos veces en el mismo bloque |
| Operación aritmética entre tipos incompatibles | por ejemplo `"total: " + 5` o `True + 1` |
| No se puede asignar String a Integer | el valor no es del tipo de la variable |
| 'f' espera 1 argumento(s) y recibió 2 | una llamada con más o menos argumentos |
| 'x' no es una función | se usan paréntesis con algo que no es función |
| Condición de If debe ser Boolean | la condición de `If`, `While` o `For` no es verdadero o falso |
| Return fuera de función | un `Return` en el programa principal |

```qmr error=3000
Integer total = 10;
Print("total: " + total);
```

**Cómo arreglarlo:** lee el mensaje y la columna marcada. En el ejemplo, `Print` ya separa sus valores, así que basta con `Print("total:", total);`.

```qmr error=3000
Integer doble(Integer n)
  Return n * 2;
End
Print(doble(1, 2));
```

## Errores de ejecución

Pasan mientras el programa corre: lo que ya se imprimió queda en la salida.

### 4001 · División por cero

```qmr error=4001
Integer cero = 0;
Print(10 / cero);
```

**Cómo arreglarlo:** revisa el divisor antes de dividir: `If (cero != 0) … End`. El `%` también da este error con cero.

### 4002 · Índice fuera de rango

Un arreglo o lista se usó con una posición que no existe. Las posiciones van de `0` a `tamaño - 1`.

```qmr error=4002
Integer a[3] {1, 2, 3};
Print(a[3]);
```

**Cómo arreglarlo:** en un `For`, usa `i < tamaño` (no `<=`). En las listas, revisa `l.size()` antes de leer o quitar.

### 4003 · Operación no válida

Una operación que no se puede hacer con esos valores. La más común: `Read` recibió algo que no se puede convertir al tipo de la variable.

```qmr error=4003 input=hola
Integer edad;
Read(edad);
```

**Cómo arreglarlo:** escribe en la entrada un dato del tipo correcto (aquí, un número).

### 4004 · Demasiadas llamadas anidadas

Una función se llamó a sí misma más de 400 veces seguidas, casi siempre por una recursión sin caso base.

```qmr error=4004
Integer cuenta(Integer n)
  Return cuenta(n + 1);
End
Print(cuenta(1));
```

**Cómo arreglarlo:** agrega el caso que detiene la recursión (un `If` que hace `Return` sin volver a llamar), o cambia la recursión por un ciclo.

### 4006 · No hay más datos de entrada

Un `Read` no encontró más líneas en la entrada.

```qmr error=4006
Integer n;
Read(n);
```

**Cómo arreglarlo:** escribe un dato por cada `Read` en el cuadro **Entrada**, uno por línea.

## Otros códigos

Estos códigos existen en el intérprete pero casi no aparecen al usar el sitio:

| Código | Mensaje |
|---|---|
| 1002 | Cadena sin cerrar (hoy se informa como [1006](#e1006)) |
| 1008 | Secuencia de escape inválida |
| 2003 – 2008 | Se esperaba `)`, `}`, `]`, un identificador, una expresión o un tipo |
| 3001 – 3006 | Variantes de errores semánticos (nombre no declarado, tipos, argumentos) |
| 4005 | Valor nulo accedido |
| 5001 – 5004 | No se pudo abrir el archivo, falta memoria o falló la lectura (al usar el intérprete en tu máquina) |
