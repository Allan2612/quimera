<picture>
  <source media="(prefers-color-scheme: dark)" srcset="web/public/logo-oscuro.svg" />
  <img src="web/public/logo-claro.svg" alt="Quimera" width="320" />
</picture>

# Quimera

Quimera es un lenguaje de programación con palabras clave en inglés y mensajes en español, hecho desde cero en C: lexer, parser, analizador semántico e intérprete. Se puede probar en el navegador, sin instalar nada, en **[quimera.avt.codes](https://quimera.avt.codes)**.

*Quimera is a small programming language written from scratch in C (lexer, parser, semantic analyzer and tree-walking interpreter). Try it in the browser at quimera.avt.codes.*

```
Integer factorial(Integer n)
  If (n <= 1)
    Return 1;
  End
  Return n * factorial(n - 1);
End

Integer List numeros {3, 5, 8};
numeros.append(13);

For (Integer i = 0 : i < numeros.size() : i += 1)
  Print("factorial de", numeros[i], "=", factorial(numeros[i]));
End
```

## Qué tiene el lenguaje

- Tipos `Integer`, `Float`, `String`, `Char`, `Boolean` y `Null`.
- Arreglos de tamaño fijo (`Integer a[5];`) y listas (`Integer List l;` con `append`, `remove` y `size`).
- `If` / `ElseIf` / `Else`, `While` y `For (inicio : condición : paso)`.
- Funciones con parámetros tipados, `Return` y recursión.
- `Print` y `Read`.
- Errores con línea y columna en cada etapa (léxica, sintáctica, semántica y de ejecución), en formato JSON para el IDE y el sitio.

La sintaxis completa está en [`interprete/Docs/Sintaxis.md`](interprete/Docs/Sintaxis.md).

## Estructura

| Carpeta | Qué es |
|---|---|
| [`interprete/`](interprete) | El lenguaje en C11. `make` compila, `make test` corre las pruebas unitarias y `make test-programs` corre los programas de ejemplo con su salida esperada. |
| [`ide/`](ide) | IDE de escritorio en Python + PyQt5. Usa el intérprete de `../interprete`. |
| [`web/`](web) | El sitio de [quimera.avt.codes](https://quimera.avt.codes): un editor en el navegador que ejecuta el mismo intérprete compilado a WebAssembly, con [documentación](https://quimera.avt.codes/docs/), un [tutorial](https://quimera.avt.codes/aprende/) y [retos](https://quimera.avt.codes/retos/) con revisión automática. |

## Cómo correrlo

**Intérprete** (gcc y make; en Windows, con MSYS2):

```sh
cd interprete
make
bin/Quimera programa.qmr
```

**Sitio web** (Node 22.18 o más nuevo):

```sh
cd web
npm install
npm run dev        # sitio local
npm test           # ejemplos, documentación, lecciones y soluciones de los retos con el intérprete en WebAssembly
npm run test:e2e   # recorridos en el navegador con Playwright (después de npm run build)
```

**IDE de escritorio:** ver [`ide/README.md`](ide/README.md).

## Autores

Proyecto del curso **EIF400 Paradigmas de Programación** de la Universidad Nacional de Costa Rica (2025), hecho por **Raúl Bejarano Mora** y **Allan Vargas Torres**. El historial de los dos repos originales se conserva en este.

Licencia [MIT](LICENSE).
