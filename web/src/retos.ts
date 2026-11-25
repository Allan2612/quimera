// Retos (/retos/): enunciado, editor y "Revisar", que corre el programa con cada caso de prueba y
// compara la salida. El código de cada reto y los resueltos se guardan en este navegador.
import './styles.css';
import { CHALLENGES, normalize, type Challenge } from './challenges/challenges';
import { createEditor, setCode } from './editor';
import { parseOutput } from './messages';
import { OutputPanel } from './output-panel';
import { createRunner } from './runner';
import { highlightStaticCode, initSite } from './site';

initSite();

const $ = <T extends HTMLElement>(sel: string) => document.querySelector(sel) as T;
const SOLVED_KEY = 'quimera.retos.resueltos';
const codeKey = (id: string) => `quimera.retos.codigo.${id}`;

const storage = {
  get(key: string) {
    try {
      return localStorage.getItem(key);
    } catch {
      return null;
    }
  },
  set(key: string, value: string) {
    try {
      localStorage.setItem(key, value);
    } catch {
      /* sin almacenamiento: el progreso dura hasta recargar */
    }
  },
};

const solved = new Set<string>(JSON.parse(storage.get(SOLVED_KEY) ?? '[]'));
const list = $<HTMLOListElement>('#challenges');
const checks = $<HTMLOListElement>('#checks');
const verdict = $<HTMLParagraphElement>('#verdict');
const checkButton = $<HTMLButtonElement>('#check');
const tryButton = $<HTMLButtonElement>('#try');

const escape = (s: string) => s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
const LEVEL_TAG: Record<Challenge['level'], string> = { Fácil: 'tag-green', Medio: 'tag-yellow', Difícil: 'tag-red' };

function paintList() {
  list.innerHTML = CHALLENGES.map(
    (c, i) =>
      `<li><a href="#${c.id}"${i === current ? ' aria-current="page"' : ''}><span class="step-n">${i + 1}</span>${c.title}${
        solved.has(c.id) ? '<span class="done" title="Resuelto" aria-label="(resuelto)">✓</span>' : ''
      }</a></li>`,
  ).join('');
  $('#progress').textContent = `· ${solved.size} de ${CHALLENGES.length}`;
}

const indexOf = (hash: string) => Math.max(0, CHALLENGES.findIndex((c) => c.id === hash.slice(1)));
let current = indexOf(location.hash);

let saveTimer: number | undefined;
/** Guarda ya el código del reto actual (al revisar, ejecutar o salir de la página) */
function saveNow() {
  clearTimeout(saveTimer);
  storage.set(codeKey(CHALLENGES[current].id), view.state.doc.toString());
}
const view = createEditor({
  parent: $('#editor'),
  doc: '',
  label: 'Tu solución',
  onRun: () => check(),
  onChange: () => {
    clearTimeout(saveTimer);
    saveTimer = window.setTimeout(saveNow, 400);
  },
});
window.addEventListener('pagehide', saveNow);
const output = new OutputPanel($('#output'), $('#status'), view, 'Ejecuta para ver la salida con el primer ejemplo, o revisa con todos los casos.');
const runner = createRunner();

function show(i: number) {
  if (saveTimer !== undefined) saveNow();
  current = i;
  const ch = CHALLENGES[i];
  const level = $('#level');
  level.textContent = ch.level;
  level.className = `tag ${LEVEL_TAG[ch.level]}`;
  $('#title').textContent = ch.title;
  const statement = $('#statement');
  statement.innerHTML = ch.statement;
  highlightStaticCode(statement);
  const visible = ch.tests.filter((t) => !t.hidden);
  $('#examples').innerHTML = `<table class="examples"><thead><tr><th>Entrada</th><th>Salida esperada</th></tr></thead><tbody>${visible
    .map((t) => `<tr><td><pre>${escape(t.input) || '<em>(vacía)</em>'}</pre></td><td><pre>${escape(t.output) || '<em>(nada)</em>'}</pre></td></tr>`)
    .join('')}</tbody></table>`;
  setCode(view, storage.get(codeKey(ch.id)) ?? ch.starter);
  output.clear();
  checks.replaceChildren();
  verdict.textContent = '';
  verdict.className = 'verdict';
  document.title = `${ch.title} · Retos de Quimera`;
  paintList();
}

async function tryFirst() {
  saveNow();
  tryButton.disabled = true;
  output.running();
  const first = CHALLENGES[current].tests.find((t) => !t.hidden) ?? CHALLENGES[current].tests[0];
  output.show(await runner.run(view.state.doc.toString(), first.input));
  tryButton.disabled = false;
}

async function check() {
  const ch = CHALLENGES[current];
  const code = view.state.doc.toString();
  saveNow();
  checkButton.disabled = true;
  checks.replaceChildren();
  verdict.className = 'verdict';
  verdict.textContent = 'Revisando…';
  let passed = 0;
  for (const [i, t] of ch.tests.entries()) {
    const outcome = await runner.run(code, t.input);
    const parsed = parseOutput(outcome.lines);
    const got = normalize(parsed.outputs.join('\n'));
    const ok = !outcome.timedOut && !outcome.failed && outcome.exitCode === 0 && got === normalize(t.output);
    if (ok) passed++;
    const li = document.createElement('li');
    li.className = ok ? 'pass' : 'fail';
    let why = '';
    if (!ok) {
      if (outcome.timedOut) why = 'Se pasó de 5 s: ¿un ciclo que no termina?';
      else if (outcome.exitCode !== 0) why = parsed.diagnostics[0]?.message || parsed.summary || 'El programa terminó con un error.';
      else why = 'La salida no es la esperada.';
    }
    const detail =
      t.hidden || ok
        ? ''
        : `<div class="diff"><div><small>Entrada</small><pre>${escape(t.input) || '(vacía)'}</pre></div><div><small>Esperado</small><pre>${escape(t.output) || '(nada)'}</pre></div><div><small>Obtenido</small><pre>${escape(got) || '(nada)'}</pre></div></div>`;
    li.innerHTML = `<span class="mark" aria-hidden="true">${ok ? '✓' : '✗'}</span><span class="what">Caso ${i + 1}${t.hidden ? ' (oculto)' : ''}: ${ok ? 'pasa' : 'falla'}${why ? ` · ${escape(why)}` : ''}</span>${detail}`;
    checks.append(li);
  }
  const all = passed === ch.tests.length;
  verdict.className = `verdict ${all ? 'ok' : 'error'}`;
  verdict.textContent = all ? `¡Reto resuelto! Pasan los ${passed} casos.` : `Pasan ${passed} de ${ch.tests.length} casos.`;
  if (all && !solved.has(ch.id)) {
    solved.add(ch.id);
    storage.set(SOLVED_KEY, JSON.stringify([...solved]));
    paintList();
  }
  checkButton.disabled = false;
}

window.addEventListener('hashchange', () => {
  show(indexOf(location.hash));
  $('#title').scrollIntoView({ block: 'start' });
});
$('#reset').addEventListener('click', () => {
  setCode(view, CHALLENGES[current].starter);
  output.clear();
});
tryButton.addEventListener('click', tryFirst);
checkButton.addEventListener('click', check);

show(current);
runner.warm();
