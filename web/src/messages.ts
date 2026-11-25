// Lectura de la salida del intérprete: cada línea es un mensaje JSON (ver interprete/Docs/Comunicacion_IDE.md)

export interface Diagnostic {
  stage?: string;
  severity?: string;
  code?: number;
  line?: number;
  column?: number;
  length?: number;
  message?: string;
}

export interface ParsedOutput {
  outputs: string[];        // lo que imprimió el programa, en orden
  diagnostics: Diagnostic[];
  summary: string;          // mensaje general de error, si lo hubo
}

export const STAGES: Record<string, string> = {
  lexer: 'Léxico',
  parser: 'Sintaxis',
  semantic: 'Semántica',
  runtime: 'Ejecución',
  system: 'Sistema',
};

export function parseOutput(lines: string[]): ParsedOutput {
  const parsed: ParsedOutput = { outputs: [], diagnostics: [], summary: '' };
  for (const line of lines) {
    let msg: { type?: string; content?: string; message?: string } & Diagnostic;
    try {
      msg = JSON.parse(line);
    } catch {
      parsed.outputs.push(line); // texto que no viene en JSON: se trata como salida
      continue;
    }
    if (msg.type === 'output') parsed.outputs.push(msg.content ?? '');
    else if (msg.type === 'diagnostic') parsed.diagnostics.push(msg);
    else if (msg.type === 'runtime_error') parsed.summary = msg.message ?? '';
  }
  return parsed;
}

/** Enlace a la explicación del error en la documentación */
export const errorDocUrl = (code?: number) => (code ? `/docs/errores/#e${code}` : '/docs/errores/');
