---
description: Cómo funciona el intérprete de Quimera por dentro - lexer, parser, analizador semántico, intérprete, el formato de los mensajes y cómo corre en el navegador con WebAssembly.
---
# Cómo funciona por dentro

Quimera está escrito en C11, sin librerías externas. Un programa pasa por cuatro etapas; cada una tiene sus propias pruebas.

```
texto ─▶ Lexer ─▶ tokens ─▶ Parser ─▶ árbol (AST) ─▶ Analizador ─▶ árbol revisado ─▶ Intérprete ─▶ salida
```

## Lexer

Lee el texto carácter por carácter y lo convierte en **tokens**: palabras clave, nombres, números, textos, operadores y delimitadores. Guarda la línea y la columna de cada uno, que después usan todos los mensajes de error. Los comentarios se descartan aquí.

## Parser

Arma el **árbol de sintaxis abstracta** (AST). Las sentencias (`If`, `While`, `For`, declaraciones, funciones) se leen por **descenso recursivo**, una función por cada una. Las expresiones usan un parser **por precedencia** (estilo Pratt): cada operador tiene su prioridad y así `3 + 4 * 2` queda como `3 + (4 * 2)`.

Cuando encuentra un error no se detiene: entra en **modo pánico**, salta hasta un punto donde puede retomar y sigue. Así un solo intento muestra varios errores, no solo el primero.

## Analizador semántico

Recorre el árbol **antes de ejecutar nada** y revisa:

- que cada nombre esté declarado y no se repita en el mismo bloque, con una tabla de símbolos por bloque;
- que los tipos combinen en cada operación, asignación, condición y llamada;
- que cada llamada tenga los argumentos correctos.

Si encuentra errores, el programa no se ejecuta. Por eso un error de tipos aparece aunque esté en una línea a la que el programa nunca llegaría.

## Intérprete

Recorre el árbol revisado y lo ejecuta nodo por nodo (un *tree-walking interpreter*). Lleva las variables de cada bloque, las llamadas a funciones con su propio espacio y un límite de 400 llamadas anidadas, para que una recursión sin fin termine con un error claro en vez de agotar la memoria.

## Los mensajes

El intérprete escribe **una línea JSON por mensaje**. Lo que imprime el programa va por la salida estándar y los errores por la de error, así el IDE y este sitio los separan sin adivinar.

```json
{"type":"output","content":"Hola, mundo","output_type":"text"}
{"type":"diagnostic","stage":"semantic","severity":"error","code":3000,"line":2,"column":7,"length":5,"message":"Identificador no declarado: 'total'"}
```

Con `line`, `column` y `length` el editor subraya el lugar exacto del error, y con `code` enlaza a [su explicación](/docs/errores/).

## En el navegador

El mismo código en C se compila a **WebAssembly** con [Emscripten](https://emscripten.org/), sin cambiar el intérprete:

- Cada ejecución corre en un **Web Worker**, fuera del hilo de la página. Si un programa pasa de 5 segundos, se descarta el worker y la página sigue respondiendo.
- Cada ejecución crea una instancia nueva del intérprete, con la memoria limpia.
- El programa se escribe en un archivo virtual, y lo que se escribe en **Entrada** llega a `Read` como si fuera el teclado.
- Nada sale del navegador: no hay servidor que ejecute el código.

El CI compila el intérprete, corre sus pruebas y comprueba que el WebAssembly del sitio sea el del código actual.
