// Ejecuta un programa de Quimera con el intérprete compilado a WebAssembly. Lo usan el Web Worker
// del sitio y las pruebas en Node. Cada ejecución crea una instancia nueva (memoria limpia).
import createQuimera from './wasm/quimera.mjs';

export interface RunResult {
  lines: string[];      // stdout y stderr en orden; cada línea es un mensaje JSON del intérprete
  exitCode: number;     // 0 si terminó bien
  truncated: boolean;   // se cortó la salida por ser demasiado larga
}

const MAX_LINES = 5000;

export async function runQuimera(source: string, input: string): Promise<RunResult> {
  const lines: string[] = [];
  let truncated = false;
  const push = (line: string) => {
    if (lines.length < MAX_LINES) lines.push(line);
    else truncated = true;
  };
  const stdin = new TextEncoder().encode(input.endsWith('\n') || input === '' ? input : input + '\n');
  let pos = 0;

  const mod = await createQuimera({
    print: push,
    printErr: push,
    noInitialRun: true,
    stdin: () => (pos < stdin.length ? stdin[pos++] : null),
  });
  mod.FS.writeFile('/programa.qmr', source);

  let exitCode = 0;
  try {
    exitCode = mod.callMain(['/programa.qmr']);
  } catch (e) {
    // Con EXIT_RUNTIME, exit() se lanza como ExitStatus
    const status = (e as { status?: number }).status;
    if (typeof status === 'number') exitCode = status;
    else {
      push(JSON.stringify({ type: 'runtime_error', message: 'El intérprete se detuvo: ' + String(e) }));
      exitCode = 1;
    }
  }
  return { lines, exitCode, truncated };
}
