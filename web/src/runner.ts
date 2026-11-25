// Ejecuta programas en un Web Worker con tiempo límite. Lo usan el editor principal, el tutorial
// y los retos. Si un programa no termina a tiempo, se descarta el worker y se crea otro.
import type { RunResult } from './run-quimera';

export interface RunOutcome extends RunResult {
  ms: number;
  timedOut: boolean;
  failed: boolean; // no se pudo cargar el intérprete
}

export const TIME_LIMIT_MS = 5000;

export function createRunner(timeLimitMs = TIME_LIMIT_MS) {
  let worker: Worker | null = null;
  let nextId = 0;

  const spawn = () => {
    worker = new Worker(new URL('./worker.ts', import.meta.url), { type: 'module' });
    return worker;
  };
  const kill = () => {
    worker?.terminate();
    worker = null;
  };

  function run(source: string, input: string): Promise<RunOutcome> {
    const id = ++nextId;
    const w = worker ?? spawn();
    return new Promise((resolve) => {
      const done = (outcome: RunOutcome) => {
        clearTimeout(timer);
        w.removeEventListener('message', onMessage);
        w.removeEventListener('error', onError);
        resolve(outcome);
      };
      const empty = { lines: [], exitCode: 1, truncated: false, ms: timeLimitMs };
      const onMessage = (e: MessageEvent<RunResult & { id: number; ms: number }>) => {
        if (e.data.id === id) done({ ...e.data, timedOut: false, failed: false });
      };
      const onError = () => {
        kill();
        done({ ...empty, ms: 0, timedOut: false, failed: true });
      };
      const timer = window.setTimeout(() => {
        kill(); // corta el programa que no termina
        done({ ...empty, timedOut: true, failed: false });
      }, timeLimitMs);
      w.addEventListener('message', onMessage);
      w.addEventListener('error', onError);
      w.postMessage({ id, source, input });
    });
  }

  /** Carga el intérprete de antemano para que la primera ejecución no espere la descarga */
  function warm() {
    (worker ?? spawn()).postMessage({ id: -1, warm: true });
  }

  return { run, warm };
}
