---
description: Qué es Quimera, cómo escribir tu primer programa y cómo ejecutarlo en el navegador, en la terminal o en el IDE.
---
# Inicio rápido

Quimera es un lenguaje de programación pequeño, con palabras clave en inglés y mensajes en español. Tiene su propio lexer, parser, analizador semántico e intérprete, escritos en C. En este sitio corre ese mismo intérprete compilado a WebAssembly, así que no hace falta instalar nada.

## Tu primer programa

```qmr
Print("Hola, mundo");
```

Cada sentencia termina en `;`. `Print` muestra lo que le pases; si le das varios valores, los separa con un espacio:

```qmr
String nombre = "Ana";
Integer edad = 21;
Print("Hola,", nombre, "tienes", edad, "años");
```

## Tres reglas para empezar

1. **Las variables tienen tipo.** Se declaran con su tipo delante: `Integer`, `Float`, `String`, `Char` o `Boolean`.
2. **Los bloques se cierran con `End`.** `If`, `While`, `For` y las funciones no usan llaves.
3. **Las palabras clave empiezan con mayúscula:** `If`, `Else`, `While`, `Return`, `True`, `Null`…

```qmr
Integer total = 0;

For (Integer i = 1 : i <= 5 : i += 1)
  total += i;
End

If (total > 10)
  Print("La suma es grande:", total);
Else
  Print("La suma es chica:", total);
End
```

## Ejecutarlo en el navegador

- Escribe en el editor de la [portada](/#probar) y pulsa **Ejecutar** (o `Ctrl` + `Enter`, `⌘` + `Enter` en Mac).
- Si el programa usa `Read`, escribe los datos en **Entrada**, una línea por cada `Read`.
- **Copiar enlace** guarda el programa en el enlace: quien lo abra ve el mismo código. El código va en el enlace, no se sube a ningún servidor.
- Si algo falla, la salida dice la etapa, la línea y la columna, y el error queda subrayado en el editor. Cada error enlaza a [su explicación](/docs/errores/).
- Un programa que pasa de 5 segundos se detiene solo, por si hay un ciclo que no termina.

Cada ejemplo de esta documentación tiene un botón **Abrir en el editor** para probarlo y cambiarlo.

## Y después

- [Referencia del lenguaje](/docs/referencia/): tipos, operadores, control de flujo, funciones, arreglos y listas.
- [Errores](/docs/errores/): qué significa cada código y cómo arreglarlo.
- [Usarlo en tu máquina](/docs/en-tu-maquina/): compilar el intérprete, usar la terminal y el IDE de escritorio.
