// Editor de Quimera (CodeMirror): tema del sitio, colores, autocompletado y marcas de error.
import { basicSetup } from 'codemirror';
import { EditorState, type Extension } from '@codemirror/state';
import { EditorView, keymap } from '@codemirror/view';
import { indentWithTab } from '@codemirror/commands';
import { indentUnit } from '@codemirror/language';
import { setDiagnostics, type Diagnostic as CmDiagnostic } from '@codemirror/lint';
import {
  snippetCompletion,
  type Completion,
  type CompletionContext,
  type CompletionResult,
} from '@codemirror/autocomplete';
import { quimeraHighlight, quimeraLanguage } from './quimera-language';
import type { Diagnostic } from './messages';

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
  '.cm-tooltip': { backgroundColor: 'var(--surface)', border: '1px solid var(--border)', borderRadius: '6px', color: 'var(--ink)' },
  '.cm-tooltip-autocomplete > ul > li[aria-selected]': { backgroundColor: 'var(--surface-2)', color: 'var(--ink)' },
  '.cm-completionDetail': { color: 'var(--muted)', fontStyle: 'normal', marginLeft: '8px' },
});

/* ---------------- Autocompletado ---------------- */

const word = (label: string, type: string, detail: string): Completion => ({ label, type, detail });

const KEYWORDS: Completion[] = [
  ...['If', 'ElseIf', 'Else', 'End', 'While', 'For', 'Return'].map((w) => word(w, 'keyword', 'control')),
  word('Print', 'function', 'muestra valores'),
  word('Read', 'function', 'lee una línea'),
  ...['And', 'Or', 'Not'].map((w) => word(w, 'keyword', 'lógico')),
  ...['Integer', 'Float', 'String', 'Char', 'Boolean', 'List'].map((w) => word(w, 'type', 'tipo')),
  ...['True', 'False', 'Null'].map((w) => word(w, 'constant', 'valor')),
];

const SNIPPETS: Completion[] = [
  snippetCompletion('If (${condición})\n\t${}\nEnd', { label: 'If', detail: 'bloque si', type: 'text', boost: -1 }),
  snippetCompletion('If (${condición})\n\t${}\nElse\n\t${}\nEnd', { label: 'If … Else', detail: 'bloque si / si no', type: 'text' }),
  snippetCompletion('While (${condición})\n\t${}\nEnd', { label: 'While', detail: 'ciclo mientras', type: 'text', boost: -1 }),
  snippetCompletion('For (Integer ${i} = 0 : ${i} < ${n} : ${i} += 1)\n\t${}\nEnd', { label: 'For', detail: 'ciclo con contador', type: 'text', boost: -1 }),
  snippetCompletion('${Integer} ${nombre}(${Integer x})\n\tReturn ${x};\nEnd', { label: 'función', detail: 'declarar función', type: 'text' }),
  snippetCompletion('Print(${});', { label: 'Print(…);', detail: 'imprimir', type: 'text' }),
];

const METHODS: Completion[] = [
  { label: 'append', type: 'method', detail: 'agrega al final', apply: 'append()' },
  { label: 'remove', type: 'method', detail: 'quita por posición', apply: 'remove()' },
  { label: 'size', type: 'method', detail: 'cantidad de elementos', apply: 'size()' },
];

/* Nombres declarados en el programa: variables, listas, arreglos y funciones */
function declaredNames(doc: string): Completion[] {
  const seen = new Map<string, Completion>();
  const re = /\b(Integer|Float|String|Char|Boolean|Null)\s+(?:List\s+)?([A-Za-z_]\w*)\s*(\()?/g;
  for (const m of doc.matchAll(re)) {
    const [, type, name, paren] = m;
    if (!seen.has(name)) seen.set(name, { label: name, type: paren ? 'function' : 'variable', detail: type });
  }
  return [...seen.values()];
}

function completeQuimera(ctx: CompletionContext): CompletionResult | null {
  const method = ctx.matchBefore(/\.\w*/);
  if (method) return { from: method.from + 1, options: METHODS, validFor: /^\w*$/ };
  const w = ctx.matchBefore(/[A-Za-z_]\w*/);
  if (!w || (w.from === w.to && !ctx.explicit)) return null;
  return {
    from: w.from,
    options: [...SNIPPETS, ...KEYWORDS, ...declaredNames(ctx.state.doc.toString())],
    validFor: /^\w*$/,
  };
}

/* ---------------- Editor ---------------- */

export interface EditorOptions {
  parent: HTMLElement;
  doc: string;
  label: string;
  onRun?: () => void;
  onChange?: (doc: string) => void;
  extra?: Extension[];
}

export function createEditor({ parent, doc, label, onRun, onChange, extra = [] }: EditorOptions): EditorView {
  const view: EditorView = new EditorView({
    parent,
    state: EditorState.create({
      doc,
      extensions: [
        basicSetup,
        quimeraLanguage,
        quimeraLanguage.data.of({ autocomplete: completeQuimera }),
        quimeraHighlight,
        indentUnit.of('  '),
        editorTheme,
        EditorView.lineWrapping,
        keymap.of([...(onRun ? [{ key: 'Mod-Enter', run: () => (onRun(), true) }] : []), indentWithTab]),
        EditorView.contentAttributes.of({ 'aria-label': label }),
        EditorView.updateListener.of((u) => {
          if (!u.docChanged) return;
          view.dispatch(setDiagnostics(view.state, []));
          onChange?.(u.state.doc.toString());
        }),
        ...extra,
      ],
    }),
  });
  return view;
}

export function setCode(view: EditorView, code: string) {
  view.dispatch({ changes: { from: 0, to: view.state.doc.length, insert: code } });
}

/** Marca en el editor los errores que reportó el intérprete (subrayado + mensaje) */
export function markErrors(view: EditorView, diagnostics: Diagnostic[]) {
  const doc = view.state.doc;
  const marks: CmDiagnostic[] = diagnostics.map((d) => {
    if (!d.line || d.line < 1) return { from: doc.length, to: doc.length, severity: 'error', message: d.message ?? '' };
    const line = doc.line(Math.min(d.line, doc.lines));
    const from = Math.min(line.from + Math.max((d.column ?? 1) - 1, 0), line.to);
    const to = Math.min(from + Math.max(d.length ?? 1, 1), line.to);
    return {
      from,
      to: to > from ? to : Math.min(from + 1, doc.length),
      severity: d.severity === 'warning' ? 'warning' : 'error',
      message: d.message ?? '',
    };
  });
  view.dispatch(setDiagnostics(view.state, marks));
}

/** Lleva el cursor a una línea y columna del programa */
export function goTo(view: EditorView, lineNo: number, column = 1) {
  const line = view.state.doc.line(Math.min(Math.max(lineNo, 1), view.state.doc.lines));
  const pos = Math.min(line.from + Math.max(column - 1, 0), line.to);
  view.dispatch({ selection: { anchor: pos }, scrollIntoView: true });
  view.focus();
}
