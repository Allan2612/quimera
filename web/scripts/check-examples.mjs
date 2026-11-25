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

// Código de las tarjetas de la página: el botón "Probar" lo ejecuta, así que debe correr sin errores
{
  const html = read(join(web, 'index.html'));
  const decode = (s) => s.replace(/&lt;/g, '<').replace(/&gt;/g, '>').replace(/&amp;/g, '&');
  for (const card of html.matchAll(/<article class="card[\s\S]*?<\/article>/g)) {
    const code = card[0].match(/<code class="qmr">([\s\S]*?)<\/code>/);
    if (!code) continue;
    const input = card[0].match(/data-input="([^"]*)"/)?.[1] ?? '';
    const title = card[0].match(/<span class="tag[^"]*">([^<]*)</)?.[1] ?? '?';
    const { lines, exitCode } = await runQuimera(decode(code[1]), input);
    const printed = lines.some((l) => l.includes('"type":"output"'));
    report('tarjeta ' + title, exitCode === 0 && printed, '  salida: ' + lines.join('\n  '));
  }
}

// Documentación: cada bloque ```qmr se ejecuta. Los normales deben terminar bien; los marcados con
// error=NNNN deben fallar con ese código (así la página de errores muestra el error que explica).
{
  const { fenceInfo } = await import('../docs-plugin.ts');
  const docs = join(web, 'src', 'docs');
  for (const f of readdirSync(docs).filter((x) => x.endsWith('.md')).sort()) {
    const md = read(join(docs, f));
    let n = 0;
    for (const m of md.matchAll(/^```(qmr[^\n]*)\n([\s\S]*?)^```$/gm)) {
      const info = fenceInfo(m[1]);
      const { lines, exitCode } = await runQuimera(m[2], info.input);
      const codes = lines.filter((l) => l.includes('"type":"diagnostic"')).map((l) => String(JSON.parse(l).code));
      const ok = info.error ? exitCode !== 0 && codes.includes(info.error) : exitCode === 0;
      report(`documentación ${f} #${++n}${info.error ? ' (error ' + info.error + ')' : ''}`, ok, '  salida: ' + lines.join('\n  '));
    }
  }
}

// Colores de sintaxis: cada palabra cae en su tipo de token
{
  const { highlightToHtml } = await import('../src/quimera-tokens.ts');
  const html = highlightToHtml(
    'If (x And True) Print(l.size(), f(2), "a") End\nInteger size = 1; /* varias\nlíneas */ Read(n);',
  );
  const has = (s) => html.includes(s);
  const checks = [
    ['If es control', has('<span class="tk-control">If</span>')],
    ['And es lógico', has('<span class="tk-logic">And</span>')],
    ['True es átomo', has('<span class="tk-atom">True</span>')],
    ['Print es E/S', has('<span class="tk-io">Print</span>')],
    ['.size es método', has('.<span class="tk-method">size</span>')],
    ['f( es función', has('<span class="tk-function">f</span>')],
    ['texto', has('<span class="tk-string">"a"</span>')],
    ['número', has('<span class="tk-number">2</span>')],
    ['size sin punto no es método', has('<span class="tk-type">Integer</span> size')],
    ['comentario de varias líneas', has('<span class="tk-comment">líneas */</span>')],
    ['después del comentario sigue coloreando', has('<span class="tk-io">Read</span>')],
  ];
  for (const [name, ok] of checks) report('sintaxis: ' + name, ok, '  html: ' + html);
}

console.log(`${total} pruebas en WebAssembly, ${fails} fallas`);
process.exit(fails ? 1 : 0);
