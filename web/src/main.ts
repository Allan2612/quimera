import './styles.css';
import { basicSetup } from 'codemirror';
import { EditorState } from '@codemirror/state';
import { EditorView, keymap } from '@codemirror/view';
import { indentWithTab } from '@codemirror/commands';
import { setDiagnostics, type Diagnostic } from '@codemirror/lint';
import { quimeraHighlight, quimeraLanguage } from './quimera-language';
import type { RunResult } from './run-quimera';
import { highlightToHtml } from './quimera-tokens';

/* Código fijo de la página (bloques y palabras dentro del texto): mismos colores que el editor */
document.querySelectorAll<HTMLElement>('main code').forEach((el) => {
  el.innerHTML = highlightToHtml(el.textContent ?? '');
});

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
const shareButton = $<HTMLButtonElement>('#share');
const stdinBox = $<HTMLTextAreaElement>('#stdin');
const output = $<HTMLOListElement>('#output');
const status = $<HTMLSpanElement>('#status');

if (!/Mac|iPhone|iPad/.test(navigator.platform)) $<HTMLElement>('#run-kbd').textContent = 'Ctrl ↵';
else $<HTMLElement>('#run-kbd').textContent = '⌘ ↵';

const CUSTOM = '__propio';
for (const ex of examples) exampleSelect.add(new Option(ex.title, ex.id));
exampleSelect.add(new Option('Tu programa', CUSTOM));

const initial = decodeShare(location.hash) ?? loadSaved();
const first = examples[0];
const startCode = initial?.code ?? first.code;
stdinBox.value = initial ? initial.input : first.input;
exampleSelect.value = initial ? (examples.find((e) => e.code === initial.code)?.id ?? CUSTOM) : first.id;

const editorTheme = EditorView.theme({
  '&': { height: '100%', fontSize: '14px', backgroundColor: 'var(--surface)', color: 'var(--ink)' },
  '.cm-scroller': { fontFamily: 'var(--font-mono)', lineHeight: '1.6' },
  '.cm-content': { padding: '14px 0', caretColor: 'var(--ink)' },
  '.cm-gutters': { backgroundColor: 'var(--surface)', color: 'var(--muted)', border: 'none' },
  '.cm-activeLine': { backgroundColor: 'color-mix(in srgb, var(--surface-2) 70%, transparent)' },
  '.cm-activeLineGutter': { backgroundColor: 'transparent', color: 'var(--ink)' },
  '.cm-cursor': { borderLeftColor: 'var(--ink)' },
  '&.cm-focused': { outline: 'none' },
  '&.cm-focused .cm-selectionBackground, .cm-selectionBackground': { backgroundColor: 'var(--selection) !important' },
  '.cm-lintRange-error': { backgroundImage: 'none', textDecoration: 'underline wavy var(--red)', textUnderlineOffset: '3px' },
  '.cm-tooltip': { backgroundColor: 'var(--surface)', border: '1px solid var(--border)', borderRadius: '6px' },
});

let saveTimer: number | undefined;
const view = new EditorView({
  parent: $<HTMLDivElement>('#editor'),
  state: EditorState.create({
    doc: startCode,
    extensions: [
      basicSetup,
      quimeraLanguage,
      quimeraHighlight,
      editorTheme,
      EditorView.lineWrapping,
      keymap.of([
        { key: 'Mod-Enter', run: () => (run(), true) },
        indentWithTab,
      ]),
      EditorView.contentAttributes.of({ 'aria-label': 'Código de Quimera' }),
      EditorView.updateListener.of((u) => {
        if (!u.docChanged) return;
        view.dispatch(setDiagnostics(view.state, []));
        if (exampleSelect.value !== CUSTOM) {
          const ex = examples.find((e) => e.id === exampleSelect.value);
          if (ex && u.state.doc.toString() !== ex.code) exampleSelect.value = CUSTOM;
        }
        clearTimeout(saveTimer);
        saveTimer = window.setTimeout(() => save(u.state.doc.toString(), stdinBox.value), 400);
      }),
    ],
  }),
});

exampleSelect.addEventListener('change', () => {
  const ex = examples.find((e) => e.id === exampleSelect.value);
  if (!ex) return;
  view.dispatch({ changes: { from: 0, to: view.state.doc.length, insert: ex.code } });
  exampleSelect.value = ex.id;
  stdinBox.value = ex.input;
  clearOutput();
});

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

function flash(button: HTMLButtonElement, text: string) {
  const original = button.textContent;
  button.textContent = text;
  setTimeout(() => (button.textContent = original), 1800);
}

/* ---------------- Ejecución ---------------- */

const TIME_LIMIT_MS = 5000;
let worker: Worker | null = null;
let runId = 0;
let timer: number | undefined;

function getWorker(): Worker {
  if (!worker) {
    worker = new Worker(new URL('./worker.ts', import.meta.url), { type: 'module' });
    worker.onmessage = (e: MessageEvent<RunResult & { id: number; ms: number }>) => {
      if (e.data.id !== runId) return;
      clearTimeout(timer);
      finish(e.data);
    };
    worker.onerror = () => {
      clearTimeout(timer);
      resetWorker();
      setBusy(false);
      showNote('No se pudo cargar el intérprete. Recarga la página e inténtalo de nuevo.');
      setStatus('error', 'Error');
    };
  }
  return worker;
}

function resetWorker() {
  worker?.terminate();
  worker = null;
}

function run() {
  const id = ++runId;
  clearOutput();
  setBusy(true);
  setStatus('', 'Ejecutando…');
  view.dispatch(setDiagnostics(view.state, []));
  getWorker().postMessage({ id, source: view.state.doc.toString(), input: stdinBox.value });
  clearTimeout(timer);
  timer = window.setTimeout(() => {
    if (id !== runId) return;
    resetWorker(); // corta el programa
    setBusy(false);
    showNote(`El programa pasó de ${TIME_LIMIT_MS / 1000} s y se detuvo. ¿Hay un ciclo que no termina?`);
    setStatus('error', 'Detenido');
  }, TIME_LIMIT_MS);
}

runButton.addEventListener('click', run);

interface Message {
  type?: string;
  content?: string;
  message?: string;
  stage?: string;
  severity?: string;
  code?: number;
  line?: number;
  column?: number;
  length?: number;
}

const STAGES: Record<string, string> = {
  lexer: 'Léxico',
  parser: 'Sintaxis',
  semantic: 'Semántica',
  runtime: 'Ejecución',
  system: 'Sistema',
};

function finish(result: RunResult & { ms: number }) {
  setBusy(false);
  const diagnostics: Diagnostic[] = [];
  let printed = 0;
  let summary = '';

  for (const line of result.lines) {
    let msg: Message;
    try {
      msg = JSON.parse(line);
    } catch {
      addLine(line); // texto que no viene en JSON: se muestra tal cual
      printed++;
      continue;
    }
    if (msg.type === 'output') {
      addLine(msg.content ?? '');
      printed++;
    } else if (msg.type === 'diagnostic') {
      addDiagnostic(msg);
      const d = toEditorDiagnostic(msg);
      if (d) diagnostics.push(d);
    } else if (msg.type === 'runtime_error') {
      summary = msg.message ?? '';
    }
  }

  if (result.truncated) showNote('La salida es muy larga: se muestran las primeras 5000 líneas.');
  if (result.exitCode === 0 && printed === 0) showNote('El programa terminó sin imprimir nada.');
  if (result.exitCode !== 0 && !output.querySelector('.diag') && summary) showNote(summary);

  view.dispatch(setDiagnostics(view.state, diagnostics));
  const time = result.ms < 1000 ? `${Math.round(result.ms)} ms` : `${(result.ms / 1000).toFixed(1)} s`;
  if (result.exitCode === 0) setStatus('ok', `Listo · ${time}`);
  else setStatus('error', diagnostics.length === 1 ? '1 error' : `${diagnostics.length || 1} errores`);
}

function toEditorDiagnostic(msg: Message): Diagnostic | null {
  const doc = view.state.doc;
  if (!msg.line || msg.line < 1) {
    return { from: doc.length, to: doc.length, severity: 'error', message: msg.message ?? '' };
  }
  const line = doc.line(Math.min(msg.line, doc.lines));
  const from = Math.min(line.from + Math.max((msg.column ?? 1) - 1, 0), line.to);
  const to = Math.min(from + Math.max(msg.length ?? 1, 1), line.to);
  return {
    from,
    to: to > from ? to : Math.min(from + 1, doc.length),
    severity: msg.severity === 'warning' ? 'warning' : 'error',
    message: msg.message ?? '',
  };
}

function addLine(text: string) {
  const li = document.createElement('li');
  li.textContent = text;
  output.append(li);
}

function addDiagnostic(msg: Message) {
  const li = document.createElement('li');
  li.className = msg.severity === 'warning' ? 'diag warning' : 'diag';
  const stage = document.createElement('span');
  stage.className = 'stage';
  stage.textContent = STAGES[msg.stage ?? ''] ?? 'Error';
  li.append(stage);
  if (msg.line && msg.line > 0) {
    const where = document.createElement('button');
    where.type = 'button';
    where.textContent = `línea ${msg.line}, col. ${msg.column ?? 1}`;
    where.title = 'Ir a esa línea';
    where.addEventListener('click', () => {
      const line = view.state.doc.line(Math.min(msg.line!, view.state.doc.lines));
      const pos = Math.min(line.from + Math.max((msg.column ?? 1) - 1, 0), line.to);
      view.dispatch({ selection: { anchor: pos }, scrollIntoView: true });
      view.focus();
    });
    li.append(where, document.createTextNode(' — '));
  }
  li.append(document.createTextNode(msg.message ?? ''));
  output.append(li);
}

function showNote(text: string) {
  const li = document.createElement('li');
  li.className = 'note';
  li.textContent = text;
  output.append(li);
}

function clearOutput() {
  output.replaceChildren();
  const li = document.createElement('li');
  li.className = 'placeholder';
  li.textContent = 'Ejecuta el programa para ver la salida.';
  output.append(li);
  setStatus('', '');
}

function setBusy(busy: boolean) {
  runButton.disabled = busy;
  if (busy) output.querySelector('.placeholder')?.remove();
}

function setStatus(kind: '' | 'ok' | 'error', text: string) {
  status.className = kind ? `status ${kind}` : 'status';
  status.textContent = text;
}

clearOutput();

/* ---------------- Entrada suave de las secciones ---------------- */

const io = new IntersectionObserver(
  (entries) => {
    for (const entry of entries) {
      if (entry.isIntersecting) {
        entry.target.classList.add('visible');
        io.unobserve(entry.target);
      }
    }
  },
  { rootMargin: '0px 0px -8% 0px' },
);
document.querySelectorAll('.reveal').forEach((el) => io.observe(el));
