// Panel de salida: muestra lo que imprimió el programa, los errores (con enlace a la línea y a
// su explicación) y el estado. Lo comparten el editor principal y el tutorial.
import type { EditorView } from '@codemirror/view';
import { goTo, markErrors } from './editor';
import { errorDocUrl, parseOutput, STAGES, type Diagnostic } from './messages';
import type { RunOutcome } from './runner';

export class OutputPanel {
  constructor(
    private list: HTMLElement,
    private status: HTMLElement,
    private view: EditorView,
    private placeholder = 'Ejecuta el programa para ver la salida.',
  ) {
    this.clear();
  }

  clear() {
    this.list.replaceChildren();
    const li = document.createElement('li');
    li.className = 'placeholder';
    li.textContent = this.placeholder;
    this.list.append(li);
    this.setStatus('', '');
  }

  running() {
    this.list.replaceChildren();
    this.setStatus('', 'Ejecutando…');
  }

  /** Texto plano de la salida (para copiarla) */
  text(): string {
    return [...this.list.querySelectorAll('li:not(.placeholder)')].map((li) => li.textContent ?? '').join('\n');
  }

  show(outcome: RunOutcome) {
    this.list.replaceChildren();
    if (outcome.failed) {
      this.note('No se pudo cargar el intérprete. Recarga la página e inténtalo de nuevo.');
      return this.setStatus('error', 'Error');
    }
    if (outcome.timedOut) {
      this.note(`El programa pasó de ${Math.round(outcome.ms / 1000)} s y se detuvo. ¿Hay un ciclo que no termina?`);
      return this.setStatus('error', 'Detenido');
    }

    const { outputs, diagnostics, summary } = parseOutput(outcome.lines);
    for (const text of outputs) this.line(text);
    for (const d of diagnostics) this.diagnostic(d);
    if (outcome.truncated) this.note('La salida es muy larga: se muestran las primeras 5000 líneas.');
    if (outcome.exitCode === 0 && outputs.length === 0) this.note('El programa terminó sin imprimir nada.');
    if (outcome.exitCode !== 0 && diagnostics.length === 0 && summary) this.note(summary);

    markErrors(this.view, diagnostics);
    const time = outcome.ms < 1000 ? `${Math.round(outcome.ms)} ms` : `${(outcome.ms / 1000).toFixed(1)} s`;
    if (outcome.exitCode === 0) this.setStatus('ok', `Listo · ${time}`);
    else this.setStatus('error', diagnostics.length === 1 ? '1 error' : `${diagnostics.length || 1} errores`);
  }

  private line(text: string) {
    const li = document.createElement('li');
    li.textContent = text;
    this.list.append(li);
  }

  private note(text: string) {
    const li = document.createElement('li');
    li.className = 'note';
    li.textContent = text;
    this.list.append(li);
  }

  private diagnostic(d: Diagnostic) {
    const li = document.createElement('li');
    li.className = d.severity === 'warning' ? 'diag warning' : 'diag';

    const head = document.createElement('div');
    head.className = 'diag-head';
    const stage = document.createElement('span');
    stage.className = 'stage';
    stage.textContent = STAGES[d.stage ?? ''] ?? 'Error';
    head.append(stage);
    if (d.line && d.line > 0) {
      const where = document.createElement('button');
      where.type = 'button';
      where.textContent = `línea ${d.line}, col. ${d.column ?? 1}`;
      where.title = 'Ir a esa línea del código';
      where.addEventListener('click', () => goTo(this.view, d.line!, d.column ?? 1));
      head.append(where);
    }
    const help = document.createElement('a');
    help.href = errorDocUrl(d.code);
    help.textContent = '¿Qué significa?';
    help.className = 'diag-help';
    head.append(help);

    const msg = document.createElement('p');
    msg.textContent = d.message ?? '';
    li.append(head, msg);
    this.list.append(li);
  }

  private setStatus(kind: '' | 'ok' | 'error', text: string) {
    this.status.className = kind ? `status ${kind}` : 'status';
    this.status.textContent = text;
  }
}
