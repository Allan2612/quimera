// Tutorial (/aprende/): una lección a la vez, con su explicación y un editor para ejecutar y cambiar el
// ejemplo. La lección actual va en el enlace (#variables), así se puede compartir o volver a ella.
import './styles.css';
import { createEditor, setCode } from './editor';
import { LESSONS } from './learn/lessons';
import { OutputPanel } from './output-panel';
import { createRunner } from './runner';
import { highlightStaticCode, initSite } from './site';

initSite();

const $ = <T extends HTMLElement>(sel: string) => document.querySelector(sel) as T;
const list = $<HTMLOListElement>('#lessons');
const stdinBox = $<HTMLDivElement>('#stdin-box');
const stdin = $<HTMLTextAreaElement>('#stdin');
const runButton = $<HTMLButtonElement>('#run');
const prev = $<HTMLAnchorElement>('#prev');
const next = $<HTMLAnchorElement>('#next');

list.innerHTML = LESSONS.map((l, i) => `<li><a href="#${l.id}"><span class="step-n">${i + 1}</span>${l.title}</a></li>`).join('');

const indexOf = (hash: string) => Math.max(0, LESSONS.findIndex((l) => l.id === hash.slice(1)));
let current = indexOf(location.hash);

const view = createEditor({ parent: $('#editor'), doc: LESSONS[current].code, label: 'Código de la lección', onRun: () => run() });
const output = new OutputPanel($('#output'), $('#status'), view);
const runner = createRunner();

function show(i: number) {
  current = i;
  const lesson = LESSONS[i];
  $('#lesson-num').textContent = `Lección ${i + 1} de ${LESSONS.length}`;
  $('#lesson-title').textContent = lesson.title;
  const body = $('#lesson-body');
  body.innerHTML = lesson.body;
  highlightStaticCode(body);
  $('#lesson-task').textContent = lesson.task;
  setCode(view, lesson.code);
  stdin.value = lesson.input ?? '';
  stdinBox.hidden = !lesson.input;
  output.clear();
  document.title = `${lesson.title} · Aprende Quimera`;
  list.querySelectorAll('a').forEach((a, j) => (j === i ? a.setAttribute('aria-current', 'page') : a.removeAttribute('aria-current')));
  for (const [link, j] of [[prev, i - 1], [next, i + 1]] as const) {
    const target = LESSONS[j];
    link.hidden = !target;
    if (target) {
      link.href = `#${target.id}`;
      link.querySelector('span')!.textContent = target.title;
    }
  }
}

window.addEventListener('hashchange', () => {
  show(indexOf(location.hash));
  $('#lesson-title').scrollIntoView({ block: 'start' });
});
$('#reset').addEventListener('click', () => {
  setCode(view, LESSONS[current].code);
  stdin.value = LESSONS[current].input ?? '';
  output.clear();
});

async function run() {
  runButton.disabled = true;
  output.running();
  output.show(await runner.run(view.state.doc.toString(), stdin.value));
  runButton.disabled = false;
}
runButton.addEventListener('click', run);

show(current);
runner.warm();
