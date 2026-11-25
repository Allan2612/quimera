// Tipos mínimos del módulo que genera Emscripten (scripts/build-wasm.mjs)
interface QuimeraModule {
  FS: { writeFile(path: string, data: string | Uint8Array): void };
  callMain(args: string[]): number;
}

interface QuimeraOptions {
  print?: (line: string) => void;
  printErr?: (line: string) => void;
  noInitialRun?: boolean;
  stdin?: () => number | null;
}

declare const createQuimera: (options?: QuimeraOptions) => Promise<QuimeraModule>;
export default createQuimera;
