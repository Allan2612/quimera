# Quimera — Semántica (resumen de la documentación)

> Resumen fiel de la **sección Semántica** del documento de Quimera. Incluye:
> modelo de ejecución, tipos y conversiones, reglas de coherencia de tipos,
> evaluación/precedencia, alcance, funciones/return, estructuras de control,
> tipos compuestos y manejo de errores.

---

## 1) Modelo de ejecución
- El archivo principal es un **bloque raíz** y el código se ejecuta **secuencialmente** de arriba hacia abajo.
- Las **funciones** se consideran bloques independientes del principal.
- **No hay punteros**, solo valores; se permite **anidación** de estructuras de control.
- La **recursividad** está permitida con **limitaciones**.
- No se pueden usar **identificadores no declarados**; no hay **sobrecarga** de funciones.
- El programa finaliza al ejecutar la **última sentencia del bloque principal**.

---

## 2) Tipos y conversiones
**Tipos primitivos:** `Integer`, `Float`, `String`, `Char`, `Boolean`, más el literal `Null` para ausencia de valor.

**Conversiones implícitas permitidas (unidireccionales):**
- `Integer → Float` (promoción a mayor precisión).
- `Char → String` (cuando se concatena con una cadena).

> No se permite la conversión inversa ni otras conversiones no listadas.

---

## 3) Coherencia de tipos y operaciones válidas
- **Aritmética** y **lógica** siguen reglas estrictas de tipos.
- Se pueden mezclar aritmética y lógica si el resultado intermedio respeta los tipos (p.ej., `If (a + b > 10)`).
- **No es válido** tratar un booleano como número (p.ej., `x = True + 1` → error semántico).
- **Comparaciones**: sólo entre operandos del **mismo tipo**, salvo igualdad/desigualdad entre `Char` y `String`.

---

## 4) Evaluación y precedencia
- Evaluación **determinista**: cada operador tiene **prioridad** y **asociatividad** definidas.
- **Paréntesis** alteran la precedencia natural y se evalúan primero.
- **Cortocircuito** en lógicas: `A And B` no evalúa B si A es `False`; `A Or B` no evalúa B si A es `True`.
- El `-` **unario** tiene mayor prioridad que su uso binario.
- Tabla de precedencia (de mayor a menor): `() []` → `Not`/unario `-` → `* / %` → `+ -` → `< <= > >=` → `== !=` → `And` → `Or` → `= += -=`.

**Ejemplos de la documentación**
```quimera
resultado = (x + y * 2 > 10) And Not terminado
x = 3 + 4 * 2      # 11
x = (3 + 4) * 2    # 14
If (x != 0 And 10 / x > 2) Print("OK") End  # evita división entre cero por cortocircuito
```

---

## 5) Alcance y visibilidad
- Las variables **existen dentro del bloque** donde se declaran; las **globales** son visibles en todo el archivo.
- Variables sin inicialización explícita reciben **`Null` por defecto** al crearse.
- Prohibida la **redeclaración** de un identificador **en el mismo ámbito** (error fatal).

---

## 6) Funciones y `Return`
- Toda función debe **finalizar con `Return`** y el **tipo devuelto** debe coincidir con la **firma**.
- Si hay condicionales, **todos los caminos** deben conducir a un `Return` (si no, **error semántico**).
- Sólo las funciones con **retorno `Null`** pueden omitir `Return`; también pueden usarlo como **escape temprano** sin valor.

---

## 7) Semántica de estructuras de control
- **If / ElseIf / Else**: evalúa una **condición `Boolean`**; el primer bloque verdadero se ejecuta y omite el resto. Sólo el último bloque del conjunto cierra con `End`.
- **While**: evalúa condición booleana; ejecuta el **bloque** mientras sea verdadera.
- **For (init : cond : act)**: equivale a un **bucle con contador/actualización** siguiendo el orden: init → comprobación de cond → cuerpo → act → …

---

## 8) Tipos compuestos: listas y arreglos
- Acceso por **índice** y **métodos** (listas): `append`, `remove`, `size`.
- Si se intenta **acceder/eliminar** un elemento inexistente → **error semántico**.
- Si el elemento **no coincide con el tipo** declarado de la lista → **alerta** y **no se inserta**.
- **Comparación/concatenación directas** no están permitidas; usar **funciones internas/procedimientos**.

---

## 9) Manejo de errores (visión general)
- Tres fases: **léxico**, **sintáctico** y **semántico**.
- Niveles: **Alerta** (no detiene) y **Error** (detiene compilación actual). El compilador puede seguir analizando para **acumular** reportes.
- El IDE muestra: **fase**, **código**, **línea/columna** y **mensaje**.
