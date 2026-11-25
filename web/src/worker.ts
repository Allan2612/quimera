// Ejecuta los programas fuera del hilo principal: si un ciclo no termina, la página sigue
// respondiendo y el runner puede cortar este worker.
import { runQuimera } from './run-quimera';

interface RunRequest {
  id: number;
  source?: string;
  input?: string;
  warm?: boolean;
}

self.onmessage = async (event: MessageEvent<RunRequest>) => {
  const { id, source = '', input = '', warm } = event.data;
  if (warm) {
    // Descarga y compila el WebAssembly de antemano (el navegador lo deja en caché)
    await runQuimera('', '');
    return;
  }
  const started = performance.now();
  const result = await runQuimera(source, input);
  self.postMessage({ id, ...result, ms: performance.now() - started });
};
