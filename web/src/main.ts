// Página principal: el editor de Quimera con ejemplos, entrada para Read, salida y enlace para compartir.
import './styles.css';
import { createEditor, setCode } from './editor';
import { OutputPanel } from './output-panel';
import { createRunner } from './runner';
import { initSite } from './site';

initSite();

/* ---------------- Ejemplos ---------------- */

const sources = import.meta.glob('./examples/*.qmr', { query: '?raw', import: 'default', eager: true }) as Record<string, string>;
const inputs = import.meta.glob('./examples/*.in', { query: '?raw', import: 'default', eager: true }) as Record<string, string>;

const TITLES: Record<string, string> = {
  '01-hola': 'Hola, mundo',
  '02-condiciones': 'Condiciones',
  '03-ciclos': 'Ciclos',
  '04-funciones': 'Funciones y recursión',
  '05-arreglos': 'Arreglos',
  '06-listas': 'Listas',
  '07-entrada': 'Entrada con Read',
  '08-errores': 'Errores',
};

const examples = Object.keys(sources)
  .sort()
  .map((path) => {
    const id = path.replace('./examples/', '').replace('.qmr', '');
    return {
      id,
      title: TITLES[id] ?? id,
      code: sources[path].replace(/\r\n/g, '\n'),
      input: (inputs[path.replace('.qmr', '.in')] ?? '').replace(/\r\n/g, '\n').trimEnd(),
    };
  });

/* ---------------- Estado guardado (solo comodidad: si falla, se ignora) ---------------- */

const STORAGE_KEY = 'quimera.programa';

function loadSaved(): { code: string; input: string } | null {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    return raw ? JSON.parse(raw) : null;
  } catch {
    return null;
  }
}

function save(code: string, input: string) {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify({ code, input }));
  } catch {
    /* sin almacenamiento: no pasa nada */
  }
}

/* Enlace para compartir: el programa va en el fragmento (#p=…), nunca sale del navegador */
function encodeShare(code: string, input: string): string {
  const bytes = new TextEncoder().encode(JSON.stringify({ code, input }));
  let bin = '';
  bytes.forEach((b) => (bin += String.fromCharCode(b)));
  return btoa(bin).replace(/\+/g, '-').replace(/\//g, '_').replace(/=+$/, '');
}

function decodeShare(hash: string): { code: string; input: string } | null {
  const m = hash.match(/^#p=([A-Za-z0-9_-]+)$/);
  if (!m) return null;
  try {
    const bin = atob(m[1].replace(/-/g, '+').replace(/_/g, '/'));
    const data = JSON.parse(new TextDecoder().decode(Uint8Array.from(bin, (c) => c.charCodeAt(0))));
    return typeof data.code === 'string' ? { code: data.code, input: String(data.input ?? '') } : null;
  } catch {
    return null;
  }
}

/* ---------------- Interfaz ---------------- */

const $ = <T extends HTMLElement>(sel: string) => document.querySelector(sel) as T;
const exampleSelect = $<HTMLSelectElement>('#example');
const runButton = $<HTMLButtonElement>('#run');
const resetButton = $<HTMLButtonElement>('#reset');
const shareButton = $<HTMLButtonElement>('#share');
const copyButton = $<HTMLButtonElement>('#copy');
const stdinBox = $<HTMLTextAreaElement>('#stdin');
const panes = $<HTMLElement>('#panes');

$<HTMLElement>('#run-kbd').textContent = /Mac|iPhone|iPad/.test(navigator.platform) ? '⌘ ↵' : 'Ctrl ↵';

const CUSTOM = '__propio';
for (const ex of examples) exampleSelect.add(new Option(ex.title, ex.id));
exampleSelect.add(new Option('Tu programa', CUSTOM));

const initial = decodeShare(location.hash) ?? loadSaved();
let baseExample = examples.find((e) => e.code === initial?.code) ?? examples[0]; // al que vuelve "Restaurar"
stdinBox.value = initial ? initial.input : baseExample.input;
exampleSelect.value = initial && !examples.some((e) => e.code === initial.code) ? CUSTOM : baseExample.id;

let saveTimer: number | undefined;
const view = createEditor({
  parent: $<HTMLDivElement>('#editor'),
  doc: initial?.code ?? baseExample.code,
  label: 'Código de Quimera',
  onRun: () => run(),
  onChange: (doc) => {
    if (exampleSelect.value !== CUSTOM && doc !== baseExample.code) exampleSelect.value = CUSTOM;
    clearTimeout(saveTimer);
    saveTimer = window.setTimeout(() => save(doc, stdinBox.value), 400);
  },
});

const output = new OutputPanel($('#output'), $('#status'), view);
const runner = createRunner();

function loadExample(id: string) {
  const ex = examples.find((e) => e.id === id);
  if (!ex) return;
  baseExample = ex;
  setCode(view, ex.code);
  exampleSelect.value = ex.id;
  stdinBox.value = ex.input;
  output.clear();
}

exampleSelect.addEventListener('change', () => loadExample(exampleSelect.value));
resetButton.addEventListener('click', () => loadExample(baseExample.id));

stdinBox.addEventListener('input', () => save(view.state.doc.toString(), stdinBox.value));
stdinBox.addEventListener('keydown', (e) => {
  if (e.key === 'Enter' && (e.ctrlKey || e.metaKey)) {
    e.preventDefault();
    run();
  }
});

shareButton.addEventListener('click', async () => {
  const url = `${location.origin}${location.pathname}#p=${encodeShare(view.state.doc.toString(), stdinBox.value)}`;
  history.replaceState(null, '', url);
  try {
    await navigator.clipboard.writeText(url);
    flash(shareButton, 'Enlace copiado');
  } catch {
    flash(shareButton, 'Enlace en la barra');
  }
});

copyButton.addEventListener('click', async () => {
  try {
    await navigator.clipboard.writeText(output.text());
    flash(copyButton, 'Copiada');
  } catch {
    flash(copyButton, 'No se pudo');
  }
});

function flash(button: HTMLButtonElement, text: string) {
  const original = button.textContent;
  button.textContent = text;
  setTimeout(() => (button.textContent = original), 1800);
}

/* ---------------- Pestañas Editor / Salida (pantallas angostas) ---------------- */

const tabs = [...document.querySelectorAll<HTMLButtonElement>('#tabs [role="tab"]')];
function showTab(name: 'editor' | 'salida') {
  panes.dataset.tab = name;
  for (const t of tabs) t.setAttribute('aria-selected', String(t.dataset.tab === name));
}
for (const t of tabs) t.addEventListener('click', () => showTab(t.dataset.tab as 'editor' | 'salida'));
showTab('editor');

/* ---------------- Ejecución ---------------- */

async function run() {
  runButton.disabled = true;
  output.running();
  showTab('salida');
  const outcome = await runner.run(view.state.doc.toString(), stdinBox.value);
  output.show(outcome);
  runButton.disabled = false;
}

runButton.addEventListener('click', run);

/* "Probar" en las tarjetas de sintaxis: lleva ese código al editor y lo ejecuta */
document.querySelectorAll<HTMLButtonElement>('.card .try').forEach((button) => {
  button.addEventListener('click', () => {
    const code = button.closest('.card')?.querySelector('pre code')?.textContent ?? '';
    setCode(view, code + '\n');
    exampleSelect.value = CUSTOM;
    stdinBox.value = button.dataset.input ?? '';
    document.querySelector('#probar')?.scrollIntoView({ behavior: 'smooth' });
    run();
  });
});

/* Carga el intérprete mientras la página está quieta, para que la primera ejecución sea inmediata */
const idle = (cb: () => void) => ('requestIdleCallback' in window ? requestIdleCallback(cb) : setTimeout(cb, 300));
idle(() => runner.warm());
