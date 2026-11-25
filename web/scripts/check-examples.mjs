// Prueba el intérprete compilado a WebAssembly (el mismo que usa el sitio):
//  - interprete/tests/programs/*.qmr: la salida debe ser igual a su .expected
//  - src/examples/*.qmr: deben terminar bien, salvo el ejemplo de errores (que debe fallar)
import { existsSync, readFileSync, readdirSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { runQuimera } from '../src/run-quimera.ts';

const web = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const read = (p) => readFileSync(p, 'utf8').replace(/\r\n/g, '\n');
let fails = 0;
let total = 0;

function report(name, ok, detail) {
  total++;
  if (ok) console.log('PASS', name);
  else {
    fails++;
    console.log('FAIL', name, '\n' + detail);
  }
}

const programs = join(web, '..', 'interprete', 'tests', 'programs');
for (const f of readdirSync(programs).filter((x) => x.endsWith('.qmr')).sort()) {
  const base = join(programs, f.slice(0, -4));
  const input = existsSync(base + '.in') ? read(base + '.in') : '';
  // El .expected solo tiene stdout; los diagnósticos (stderr) se reconocen por su "type"
  const { lines, exitCode } = await runQuimera(read(base + '.qmr'), input);
  const stdout = lines.filter((l) => !l.includes('"type":"diagnostic"'));
  const actual = [...stdout, `exit=${exitCode}`].join('\n');
  const expected = read(base + '.expected').trimEnd();
  report('programa ' + f, actual === expected, `  esperado: ${JSON.stringify(expected)}\n  obtenido: ${JSON.stringify(actual)}`);
}

const examples = join(web, 'src', 'examples');
for (const f of readdirSync(examples).filter((x) => x.endsWith('.qmr')).sort()) {
  const base = join(examples, f.slice(0, -4));
  const input = existsSync(base + '.in') ? read(base + '.in') : '';
  const { lines, exitCode } = await runQuimera(read(base + '.qmr'), input);
  const wantError = f.includes('errores');
  report('ejemplo ' + f, wantError ? exitCode === 1 : exitCode === 0, '  salida: ' + lines.join('\n  '));
}

console.log(`${total} pruebas en WebAssembly, ${fails} fallas`);
process.exit(fails ? 1 : 0);
