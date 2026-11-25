// Lecciones del tutorial (/aprende/). Solo datos: las usa la página y también npm test, que ejecuta el
// código de cada una para que ninguna quede rota. `body` es HTML corto; `input` llena el cuadro de Entrada.

export interface Lesson {
  id: string;
  title: string;
  body: string;
  code: string;
  input?: string;
  task: string; // un cambio para practicar
}

export const LESSONS: Lesson[] = [
  {
    id: 'hola',
    title: 'Hola, mundo',
    body: `<p><code>Print</code> muestra en la salida lo que le pases entre paréntesis. Los textos van entre comillas dobles y cada sentencia termina en <code>;</code>.</p>
<p>Pulsa <strong>Ejecutar</strong> (o <kbd>Ctrl</kbd> + <kbd>Enter</kbd>) para correr el programa.</p>`,
    code: 'Print("Hola, mundo");\nPrint("Estoy aprendiendo Quimera");\n',
    task: 'Agrega una tercera línea que imprima tu nombre.',
  },
  {
    id: 'variables',
    title: 'Variables y tipos',
    body: `<p>Una variable guarda un valor. Se declara con su <strong>tipo</strong> delante: <code>Integer</code> (enteros), <code>Float</code> (decimales), <code>String</code> (textos), <code>Char</code> (un carácter) y <code>Boolean</code> (<code>True</code> o <code>False</code>).</p>
<p><code>Print</code> acepta varios valores separados por comas y los muestra con un espacio entre ellos.</p>`,
    code: 'String nombre = "Ana";\nInteger edad = 21;\nFloat altura = 1.65;\nBoolean estudia = True;\n\nPrint(nombre, "tiene", edad, "años");\nPrint("Mide", altura, "y estudia:", estudia);\n',
    task: 'Cambia la edad y agrega una variable Char con tu inicial.',
  },
  {
    id: 'operaciones',
    title: 'Operaciones',
    body: `<p>Con números se usan <code>+ - * / %</code>. Entre dos <code>Integer</code>, la división descarta los decimales y <code>%</code> da el resto. Si uno de los dos es <code>Float</code>, el resultado tiene decimales.</p>
<p><code>+=</code> y <code>-=</code> suman o restan a una variable. Con <code>+</code> también se unen textos.</p>`,
    code: 'Integer a = 17;\nInteger b = 5;\nPrint(a + b, a - b, a * b);\nPrint(a / b, a % b, a / 5.0);\n\nInteger puntos = 10;\npuntos += 5;\nPrint("Puntos:", puntos);\nPrint("Hola, " + "Quimera");\n',
    task: 'Calcula cuántos minutos hay en 3 horas y media.',
  },
  {
    id: 'condiciones',
    title: 'Condiciones',
    body: `<p><code>If</code> ejecuta un bloque solo si la condición es verdadera. <code>ElseIf</code> prueba otra condición y <code>Else</code> cubre el resto. Todo el conjunto se cierra con un solo <code>End</code>.</p>
<p>Se compara con <code>== != &lt; &lt;= &gt; &gt;=</code> y se combinan condiciones con <code>And</code>, <code>Or</code> y <code>Not</code>.</p>`,
    code: 'Integer temperatura = 24;\n\nIf (temperatura >= 30)\n  Print("Hace calor");\nElseIf (temperatura >= 18 And temperatura < 30)\n  Print("Está agradable");\nElse\n  Print("Hace frío");\nEnd\n',
    task: 'Prueba con 35 y con 10, y agrega un caso para menos de 0.',
  },
  {
    id: 'ciclos',
    title: 'Ciclos',
    body: `<p><code>While</code> repite su bloque mientras la condición sea verdadera. <code>For</code> junta en una línea el inicio, la condición y el paso, separados por <code>:</code>.</p>
<p>Si un ciclo nunca termina, el sitio lo detiene a los 5 segundos.</p>`,
    code: 'Integer n = 3;\nWhile (n > 0)\n  Print("Faltan", n);\n  n -= 1;\nEnd\n\nInteger suma = 0;\nFor (Integer i = 1 : i <= 10 : i += 1)\n  suma += i;\nEnd\nPrint("1 + 2 + ... + 10 =", suma);\n',
    task: 'Cambia el For para imprimir solo los números pares del 2 al 20.',
  },
  {
    id: 'funciones',
    title: 'Funciones',
    body: `<p>Una función agrupa código para reutilizarlo. Primero va el tipo que devuelve (o <code>Null</code> si no devuelve nada), después el nombre y los parámetros con su tipo. Se cierra con <code>End</code> y devuelve con <code>Return</code>.</p>
<p>Hay que declararla antes de usarla. Puede llamarse a sí misma (recursión).</p>`,
    code: 'Integer mayor(Integer a, Integer b)\n  If (a > b)\n    Return a;\n  End\n  Return b;\nEnd\n\nNull saludar(String nombre)\n  Print("Hola,", nombre);\nEnd\n\nPrint(mayor(8, 13));\nsaludar("Luis");\n',
    task: 'Escribe una función Integer triple(Integer n) y úsala.',
  },
  {
    id: 'colecciones',
    title: 'Arreglos y listas',
    body: `<p>Un <strong>arreglo</strong> tiene tamaño fijo y se usa por posición, empezando en 0. Una <strong>lista</strong> crece con <code>append</code>, se achica con <code>remove</code> y dice su tamaño con <code>size()</code>.</p>`,
    code: 'Integer notas[3] {80, 65, 92};\nnotas[1] = 70;\nPrint(notas[0], notas[1], notas[2]);\n\nString List frutas;\nfrutas.append("mango");\nfrutas.append("piña");\nfrutas.append("cas");\nPrint(frutas.size(), "frutas:", frutas);\n\nFor (Integer i = 0 : i < frutas.size() : i += 1)\n  Print(i, frutas[i]);\nEnd\n',
    task: 'Quita la primera fruta con frutas.remove(0) e imprime la lista otra vez.',
  },
  {
    id: 'leer',
    title: 'Leer datos',
    body: `<p><code>Read</code> lee una línea del cuadro <strong>Entrada</strong> y la guarda en una variable, convertida a su tipo. Escribe un dato por línea, uno por cada <code>Read</code>.</p>
<p>Con esto ya puedes resolver los <a href="/retos/">retos</a>: cada uno lee datos y debe imprimir la respuesta exacta.</p>`,
    code: 'String nombre;\nInteger nacimiento;\nRead(nombre);\nRead(nacimiento);\n\nInteger edad = 2026 - nacimiento;\nPrint(nombre, "cumple", edad, "este año");\n',
    input: 'Valeria\n2004',
    task: 'Cambia los datos de Entrada y vuelve a ejecutar.',
  },
];
