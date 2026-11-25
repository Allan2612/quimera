// Ejecuta los programas fuera del hilo principal: si un ciclo no termina, la página sigue
// respondiendo y main.ts puede cortar este worker.
import { runQuimera } from './run-quimera';

interface RunRequest {
  id: number;
  source: string;
  input: string;
}

self.onmessage = async (event: MessageEvent<RunRequest>) => {
  const { id, source, input } = event.data;
  const started = performance.now();
  const result = await runQuimera(source, input);
  self.postMessage({ id, ...result, ms: performance.now() - started });
};
