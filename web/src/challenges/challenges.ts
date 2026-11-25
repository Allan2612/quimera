// Retos (/retos/). Solo datos: los usa la página y también npm test, que corre la solución de referencia
// de cada uno (web/retos-soluciones/<id>.qmr, fuera del sitio) contra todos sus casos.
// En cada caso, `input` va al cuadro de Entrada (una línea por Read) y `output` es la salida exacta.

export interface TestCase {
  input: string;
  output: string;
  hidden?: boolean; // no se muestra el dato, solo si pasó
}

export interface Challenge {
  id: string;
  title: string;
  level: 'Fácil' | 'Medio' | 'Difícil';
  statement: string; // HTML corto
  starter: string;
  tests: TestCase[];
}

export const CHALLENGES: Challenge[] = [
  {
    id: 'suma',
    title: 'Suma de 1 a N',
    level: 'Fácil',
    statement: '<p>Lee un entero <code>N</code> e imprime la suma de todos los números de 1 a <code>N</code>. Si <code>N</code> es 0, la suma es 0.</p>',
    starter: 'Integer n;\nRead(n);\n\n// Tu código aquí\n',
    tests: [
      { input: '5', output: '15' },
      { input: '1', output: '1' },
      { input: '100', output: '5050', hidden: true },
      { input: '0', output: '0', hidden: true },
    ],
  },
  {
    id: 'par-impar',
    title: 'Par o impar',
    level: 'Fácil',
    statement: '<p>Lee un entero e imprime <code>par</code> o <code>impar</code>.</p>',
    starter: 'Integer n;\nRead(n);\n\n// Tu código aquí\n',
    tests: [
      { input: '4', output: 'par' },
      { input: '7', output: 'impar' },
      { input: '0', output: 'par', hidden: true },
      { input: '-3', output: 'impar', hidden: true },
    ],
  },
  {
    id: 'fizzbuzz',
    title: 'FizzBuzz',
    level: 'Fácil',
    statement:
      '<p>Lee <code>N</code> e imprime los números de 1 a <code>N</code>, uno por línea. Pero si el número es múltiplo de 3 imprime <code>Fizz</code>; si es múltiplo de 5, <code>Buzz</code>; y si es múltiplo de los dos, <code>FizzBuzz</code>.</p>',
    starter: 'Integer n;\nRead(n);\n\n// Tu código aquí\n',
    tests: [
      { input: '5', output: '1\n2\nFizz\n4\nBuzz' },
      { input: '15', output: '1\n2\nFizz\n4\nBuzz\nFizz\n7\n8\nFizz\nBuzz\n11\nFizz\n13\n14\nFizzBuzz', hidden: true },
    ],
  },
  {
    id: 'maximo',
    title: 'El mayor',
    level: 'Fácil',
    statement:
      '<p>La primera línea trae cuántos números siguen (al menos 1), y después un número por línea. Imprime el mayor.</p>',
    starter: 'Integer cantidad;\nRead(cantidad);\n\n// Lee cada número con un ciclo y guarda el mayor\n',
    tests: [
      { input: '3\n4\n9\n2', output: '9' },
      { input: '2\n-5\n-8', output: '-5' },
      { input: '5\n1\n2\n3\n4\n5', output: '5', hidden: true },
      { input: '1\n42', output: '42', hidden: true },
    ],
  },
  {
    id: 'positivos',
    title: 'Contar positivos',
    level: 'Medio',
    statement:
      '<p>La primera línea trae cuántos números siguen, y después un número por línea. Imprime cuántos son mayores que 0. El cero no cuenta.</p>',
    starter: 'Integer cantidad;\nRead(cantidad);\n\n// Tu código aquí\n',
    tests: [
      { input: '4\n3\n-1\n0\n8', output: '2' },
      { input: '3\n-2\n-7\n-1', output: '0' },
      { input: '0', output: '0', hidden: true },
      { input: '5\n1\n1\n1\n1\n1', output: '5', hidden: true },
    ],
  },
  {
    id: 'palindromo',
    title: 'Número palíndromo',
    level: 'Medio',
    statement:
      '<p>Lee un entero positivo e imprime <code>si</code> si se lee igual al derecho y al revés (como 1221), o <code>no</code> si no.</p><p>Pista: <code>n % 10</code> da el último dígito y <code>n / 10</code> lo quita.</p>',
    starter: 'Integer n;\nRead(n);\n\n// Tu código aquí\n',
    tests: [
      { input: '1221', output: 'si' },
      { input: '123', output: 'no' },
      { input: '7', output: 'si', hidden: true },
      { input: '12321', output: 'si', hidden: true },
      { input: '10', output: 'no', hidden: true },
    ],
  },
  {
    id: 'factorial',
    title: 'Factorial recursivo',
    level: 'Medio',
    statement:
      '<p>Escribe una función recursiva <code>Integer factorial(Integer n)</code>, lee <code>n</code> e imprime su factorial. El factorial de 0 es 1.</p>',
    starter: 'Integer factorial(Integer n)\n  // Tu código aquí\n  Return 1;\nEnd\n\nInteger n;\nRead(n);\nPrint(factorial(n));\n',
    tests: [
      { input: '5', output: '120' },
      { input: '0', output: '1' },
      { input: '10', output: '3628800', hidden: true },
    ],
  },
  {
    id: 'fibonacci',
    title: 'Fibonacci',
    level: 'Medio',
    statement:
      '<p>Lee <code>N</code> e imprime los primeros <code>N</code> números de Fibonacci, uno por línea. La serie empieza en 0 y 1, y cada número es la suma de los dos anteriores.</p>',
    starter: 'Integer n;\nRead(n);\n\n// Tu código aquí\n',
    tests: [
      { input: '7', output: '0\n1\n1\n2\n3\n5\n8' },
      { input: '1', output: '0' },
      { input: '10', output: '0\n1\n1\n2\n3\n5\n8\n13\n21\n34', hidden: true },
    ],
  },
  {
    id: 'primos',
    title: 'Números primos',
    level: 'Difícil',
    statement:
      '<p>Lee <code>N</code> e imprime todos los números primos menores o iguales a <code>N</code>, uno por línea. Un primo es mayor que 1 y solo se divide entre 1 y sí mismo.</p>',
    starter: 'Integer n;\nRead(n);\n\n// Tu código aquí\n',
    tests: [
      { input: '10', output: '2\n3\n5\n7' },
      { input: '2', output: '2' },
      { input: '30', output: '2\n3\n5\n7\n11\n13\n17\n19\n23\n29', hidden: true },
      { input: '1', output: '', hidden: true },
    ],
  },
  {
    id: 'ordenar',
    title: 'Ordenar',
    level: 'Difícil',
    statement:
      '<p>La primera línea trae cuántos números siguen, y después un número por línea. Guárdalos en una lista, ordénalos de menor a mayor e imprime la lista con <code>Print</code> (se ve así: <code>[1, 2, 3]</code>).</p>',
    starter: 'Integer cantidad;\nRead(cantidad);\nInteger List numeros;\n\n// Lee los números, ordénalos y luego:\nPrint(numeros);\n',
    tests: [
      { input: '4\n3\n1\n4\n2', output: '[1, 2, 3, 4]' },
      { input: '3\n5\n-2\n5', output: '[-2, 5, 5]' },
      { input: '6\n9\n8\n7\n6\n5\n4', output: '[4, 5, 6, 7, 8, 9]', hidden: true },
      { input: '1\n0', output: '[0]', hidden: true },
    ],
  },
];

/** Salida del programa lista para comparar: sin espacios al final de cada línea ni líneas vacías al final */
export const normalize = (s: string) =>
  s.replace(/\r\n/g, '\n').split('\n').map((l) => l.trimEnd()).join('\n').trim();
