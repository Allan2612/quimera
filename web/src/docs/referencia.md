---
description: Referencia del lenguaje Quimera - tipos, valores por defecto, operadores y precedencia, control de flujo, funciones, alcance, arreglos, listas, Print, Read y comentarios.
---
# Referencia del lenguaje

Todo lo que acepta Quimera, con un ejemplo que puedes abrir en el editor.

## Tipos y valores

| Tipo | Ejemplo | Valor por defecto |
|---|---|---|
| `Integer` | `42`, `-7` | `null` |
| `Float` | `3.14`, `2.0` | `null` |
| `String` | `"hola"` | `null` |
| `Char` | `'a'` | `null` |
| `Boolean` | `True`, `False` | `null` |

Una variable declarada sin valor empieza en `Null`, que se muestra como `null`:

```qmr
Integer x;
Print(x);
x = 5;
Print(x);
```

Los valores booleanos se escriben `True` y `False`, y se muestran en minúscula:

```qmr
Boolean listo = True;
Print(listo, Not listo);
```

### Conversiones automáticas

Quimera convierte un tipo en otro por su cuenta solo en dos casos:

- **`Integer` a `Float`**, cuando se mezclan en una operación o se guarda un entero en un `Float`.
- **`Char` a `String`**, al concatenar un carácter con un texto usando `+`.

```qmr
Float f = 5;
Char c = 'a';
Print(f, 5 + 2.5, "b" + c);
```

Cualquier otra mezcla es un [error de tipos](/docs/errores/#e3000): no se puede sumar un `Boolean` ni concatenar un `String` con un número.

## Operadores

| Grupo | Operadores |
|---|---|
| Aritméticos | `+` `-` `*` `/` `%` |
| Comparación | `==` `!=` `<` `<=` `>` `>=` |
| Lógicos | `And` `Or` `Not` |
| Asignación | `=` `+=` `-=` |

La división entre dos `Integer` es entera (descarta los decimales). Para obtener decimales, al menos uno de los dos tiene que ser `Float`:

```qmr
Print(7 / 2, 7.0 / 2, 7 % 2);
```

### Precedencia

De mayor a menor: paréntesis, `Not` y `-` unario, `*` `/` `%`, `+` `-`, comparaciones, `And`, `Or`. Los paréntesis cambian el orden:

```qmr
Print(3 + 4 * 2, (3 + 4) * 2);
```

`And` y `Or` se evalúan en cortocircuito: si el lado izquierdo ya decide el resultado, el derecho no se evalúa. Por eso esta división nunca se intenta:

```qmr
Integer x = 0;
If (x != 0 And 10 / x > 2)
  Print("grande");
Else
  Print("x es cero, no se divide");
End
```

## Control de flujo

Las condiciones van entre paréntesis y tienen que ser `Boolean`. Cada bloque se cierra con `End`.

### If, ElseIf y Else

```qmr
Integer nota = 65;
If (nota >= 70)
  Print("Aprobado");
ElseIf (nota >= 60)
  Print("Ampliación");
Else
  Print("Reprobado");
End
```

### While

```qmr
Integer n = 3;
While (n > 0)
  Print(n);
  n -= 1;
End
Print("¡Despegue!");
```

### For

`For (inicio : condición : paso)`: el inicio corre una vez, la condición se revisa antes de cada vuelta y el paso al final de cada una.

```qmr
For (Integer i = 0 : i < 10 : i += 3)
  Print(i);
End
```

## Funciones

El tipo de retorno va primero. Una función que no devuelve nada usa `Null`. Los parámetros llevan su tipo, y hay que declarar la función antes de usarla.

```qmr
Integer cuadrado(Integer n)
  Return n * n;
End

Null saludar(String nombre)
  Print("Hola,", nombre);
End

Print(cuadrado(4));
saludar("Ana");
```

Las funciones `Null` pueden usar `Return;` sin valor para salir antes. Si una función con tipo termina sin llegar a un `Return`, devuelve `null`, así que conviene que todos los caminos terminen en uno.

### Recursión

Una función se puede llamar a sí misma, hasta 400 llamadas anidadas. Más que eso se detiene con el [error 4004](/docs/errores/#e4004).

```qmr
Integer factorial(Integer n)
  If (n <= 1)
    Return 1;
  End
  Return n * factorial(n - 1);
End

Print(factorial(10));
```

## Alcance

Una variable existe dentro del bloque donde se declara. Las variables del nivel principal se pueden usar dentro de las funciones.

```qmr
Integer limite = 3;

Integer recortar(Integer n)
  If (n > limite)
    Return limite;
  End
  Return n;
End

Print(recortar(10), recortar(2));
```

Fuera del bloque, la variable ya no existe:

```qmr error=3000
If (True)
  Integer y = 1;
End
Print(y);
```

No se puede declarar dos veces el mismo nombre en el mismo bloque.

## Arreglos

Tamaño fijo, índices desde 0. Se pueden dar los valores iniciales entre llaves; si no, empiezan en cero (`0`, `""` o `false`).

```qmr
Integer a[3] {10, 20, 30};
a[1] += 5;
Print(a[0], a[1], a[2]);

Integer vacio[2];
Print(vacio[0]);
```

Leer o escribir fuera del tamaño es el [error 4002](/docs/errores/#e4002).

## Listas

Crecen y se achican. Se declaran con el tipo de sus elementos y `List`:

| Método | Qué hace |
|---|---|
| `l.append(valor)` | agrega al final |
| `l.remove(posición)` | quita el elemento en esa posición |
| `l.size()` | cantidad de elementos |

```qmr
String List compras;
compras.append("pan");
compras.append("leche");
compras.append("café");
compras.remove(0);

Print(compras.size(), compras[0]);
Print(compras);
```

Se accede y se asigna por índice igual que en los arreglos, y se pueden dar valores iniciales:

```qmr
Integer List l {1, 2};
l[0] = 5;
l[1] += 1;
Print(l);
```

Las listas no se pueden pasar como parámetro a una función.

## Entrada y salida

### Print

Recibe uno o más valores y los muestra en una línea, separados por un espacio. Las listas se muestran entre corchetes.

```qmr
Integer List l {1, 2, 3};
Print("texto", 1, 2.5, True, 'c', l);
```

### Read

Lee una línea de la entrada y la convierte al tipo de la variable. Un `String` recibe la línea completa, con espacios.

```qmr input=Ana María|21
String nombre;
Integer edad;
Read(nombre);
Read(edad);
Print(nombre, "cumple", edad + 1, "el próximo año");
```

Si el dato no se puede convertir (por ejemplo, letras en un `Integer`) es el [error 4003](/docs/errores/#e4003). Si se acaban los datos, el [4006](/docs/errores/#e4006).

## Comentarios

```qmr
// Comentario de una línea

/* Comentario
   de varias líneas */
Print("Los comentarios no se ejecutan");
```

## Palabras reservadas

`If` `ElseIf` `Else` `End` `While` `For` `Return` `Print` `Read` `And` `Or` `Not` `True` `False` `Null` `Integer` `Float` `String` `Char` `Boolean` `List`

No se pueden usar como nombres de variables ni de funciones.
