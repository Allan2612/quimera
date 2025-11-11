# Quimera — Sintaxis y estructuras del lenguaje (resumen)

> Compilado a partir de la sección **Sintaxis** del documento del proyecto Quimera.  
> Este archivo reúne **todas las estructuras del lenguaje** en forma breve y copiable.

---

## Notación (BNF/EBNF usada)
- `<no_terminal>`: categorías gramaticales (p. ej., `<expresion>`, `<sentencia>`).
- Literales/terminales se escriben tal cual (palabras reservadas con mayúscula inicial: `If`, `Else`, `End`, …).
- `::=` define una producción.
- `|` separa alternativas.
- `[[ … ]]` indica opcional.
- `{ { … } }` indica repetición (0+).

---

## Declaraciones
```ebnf
<declaracion_variable> ::= <tipo> <identificador> [[ = <expresion> ]] ;

<declaracion_funcion> ::= <tipo_retorno> <identificador> ( [[ <parametros> ]] ) <bloque> End

<declaracion_arreglo> ::= <tipo> <identificador> [ <expresion_aritmetica> ]
                          [[ <lista_elementos> ]] ;

<declaracion_lista>   ::= <tipo> List <identificador> [[ <lista_elementos> ]] ;

<tipo>          ::= String | Char | Integer | Float | Boolean
<tipo_retorno>  ::= <tipo> | Null
<parametros>    ::= <parametro> { { , <parametro> } }
<parametro>     ::= <tipo> <identificador>
<lista_elementos> ::= { <expresion> { { , <expresion> } } }
```

---

## Asignaciones
```ebnf
<asignacion>            ::= <identificador> = <expresion> ;

<asignacion_compuesta>  ::= <identificador> <operador_asignacion> <expresion> ;
<operador_asignacion>   ::= += | -=
```
> Nota: en operaciones sobre **listas/arreglos** (ver más abajo) `<operador_asignacion>` también admite `=`.

---

## Expresiones
```ebnf
<expresion> ::= <expresion_aritmetica> | <expresion_logica>

<expresion_aritmetica> ::= [[ <operador_aditivo> ]] <termino>
                           { { <operador_aditivo> <termino> } }
<termino>              ::= <factor> { { <operador_multiplicativo> <factor> } }
<factor>               ::= <identificador>
                         | <entero>
                         | <acceso_arreglo>
                         | <acceso_lista>
                         | <tamaño_lista>
                         | <llamada_funcion>
                         | ( <expresion_aritmetica> )
<operador_aditivo>     ::= + | -
<operador_multiplicativo> ::= * | / | **

<expresion_logica>     ::= [[ <operador_unario> ]] <expresion_relacional>
                           { { <operador_logico> <expresion_relacional> } }
<expresion_relacional> ::= <expresion_aritmetica>
                           { { <operador_relacional> <expresion_aritmetica> } }
<operador_relacional>  ::= < | > | <= | >= | == | !=
<operador_logico>      ::= And | Or
<operador_unario>      ::= Not
```

---

## Control de flujo
```ebnf
<if_stmt>    ::= If ( <expresion_logica> ) <bloque>
                 [[ ElseIf ( <expresion_logica> ) <bloque> ]]
                 [[ Else <bloque> ]]
                 End

<while_stmt> ::= While ( <expresion_logica> ) <bloque> End

<for_stmt>   ::= For ( <inicializacion> : <expresion_logica> : <actualizacion> )
                 <bloque> End
<inicializacion> ::= <declaracion_variable> | <asignacion>
<actualizacion>  ::= <asignacion> | <asignacion_compuesta>
```

---

## Uso de funciones y estructuras de datos
```ebnf
<retorno_funcion> ::= Return [[ <expresion> ]] ;

<llamada_funcion> ::= <identificador> ( [[ <argumentos> ]] ) [[;]]
<argumentos>      ::= <expresion> { { , <expresion> } }

<acceso_arreglo>  ::= <identificador> [ <expresion_aritmetica> ]
<asignacion_arreglo> ::= <identificador> [ <expresion_aritmetica> ]
                         <operador_asignacion_ls> <expresion> ;
<acceso_lista>    ::= <identificador> [ <expresion_aritmetica> ]
<asignacion_lista>::= <identificador> [ <expresion_aritmetica> ]
                         <operador_asignacion_ls> <expresion> ;
<agregar_elemento_lista> ::= <identificador>.append( <expresion_aritmetica> ) ;
<remover_elemento_lista> ::= <identificador>.remove( <expresion_aritmetica> ) ;
<tamaño_lista>    ::= <identificador>.size()

<operador_asignacion_ls> ::= += | -= | =
```
**Notas de uso**
- **Funciones expresivas** (devuelven valor) se pueden usar en expresiones y **no** llevan `;`.
- **Funciones sentenciales** (no devuelven) se usan como sentencia y **sí** llevan `;`.

---

## Sentencias y bloques
```ebnf
<sentencia> ::= <declaracion_variable>
              | <declaracion_funcion>
              | <declaracion_arreglo>
              | <declaracion_lista>
              | <asignacion>
              | <asignacion_compuesta>
              | <asignacion_arreglo>
              | <agregar_elemento_lista>
              | <remover_elemento_lista>
              | <asignacion_lista>
              | <llamada_funcion>
              | <retorno_funcion>
              | <if_stmt>
              | <while_stmt>
              | <for_stmt>
              | <input_stmt>
              | <output_stmt>

<bloque>    ::= { { <sentencia> } }
```
> `End` **no** forma parte de `<bloque>`; aparece en las construcciones que lo requieren (If, While, Function, For).

---

## Entrada/Salida (I/O)
```ebnf
<input_stmt>  ::= Read ( <identificador> ) ;
<output_stmt> ::= Print ( <expresion> ) ;
```

---

## Observaciones importantes
- Las producciones que **terminan con `;`** son **sentencias** (no se usan como expresiones/argumentos).
- Tipos **compuestos**: arreglos (tamaño fijo) y listas (dinámicas) con métodos `append`, `remove`, `size`.
- La diferencia “**expresiva** vs **sentencial**” en llamadas a función determina si llevan `;` y dónde pueden aparecer.
