// Tokenizador de Quimera para colorear código. Es puro (sin CodeMirror ni DOM): lo usan el editor,
// los bloques de código de la página y la documentación generada en Node. Mismas palabras que el
// lexer en C (interprete/src/lexer/lexer.c).

export type TokenKind =
  | 'control'   // If ElseIf Else End While For Return
  | 'io'        // Print Read
  | 'logic'     // And Or Not
  | 'type'      // Integer Float String Char Boolean List
  | 'atom'      // True False Null
  | 'string'
  | 'number'
  | 'comment'
  | 'method'    // .append .remove .size
  | 'function'  // nombre seguido de "("
  | 'variable'
  | 'operator'
  | 'plain';

const CONTROL = new Set(['If', 'ElseIf', 'Else', 'End', 'While', 'For', 'Return']);
const IO = new Set(['Print', 'Read']);
const LOGIC = new Set(['And', 'Or', 'Not']);
const TYPES = new Set(['Integer', 'Float', 'String', 'Char', 'Boolean', 'List']);
const ATOMS = new Set(['True', 'False', 'Null']);
const METHODS = new Set(['append', 'remove', 'size']);

export interface ScanState {
  inComment: boolean;
}

export const startState = (): ScanState => ({ inComment: false });

/** Lee un token de `line` a partir de `pos`. Devuelve dónde termina y de qué tipo es. */
export function scanToken(line: string, pos: number, state: ScanState): { end: number; kind: TokenKind } {
  if (state.inComment) {
    const close = line.indexOf('*/', pos);
    if (close === -1) return { end: line.length, kind: 'comment' };
    state.inComment = false;
    return { end: close + 2, kind: 'comment' };
  }
  const rest = line.slice(pos);
  let m: RegExpMatchArray | null;

  if ((m = rest.match(/^\s+/))) return { end: pos + m[0].length, kind: 'plain' };
  if (rest.startsWith('//')) return { end: line.length, kind: 'comment' };
  if (rest.startsWith('/*')) {
    const close = line.indexOf('*/', pos + 2);
    if (close === -1) {
      state.inComment = true; // sigue en las líneas siguientes
      return { end: line.length, kind: 'comment' };
    }
    return { end: close + 2, kind: 'comment' };
  }
  if ((m = rest.match(/^"(?:[^"\\]|\\.)*"?/))) return { end: pos + m[0].length, kind: 'string' };
  if ((m = rest.match(/^'(?:[^'\\]|\\.)'?/))) return { end: pos + m[0].length, kind: 'string' };
  if ((m = rest.match(/^\d+(?:\.\d+)?/))) return { end: pos + m[0].length, kind: 'number' };
  if ((m = rest.match(/^[A-Za-z_][A-Za-z0-9_]*/))) {
    const word = m[0];
    const end = pos + word.length;
    if (CONTROL.has(word)) return { end, kind: 'control' };
    if (IO.has(word)) return { end, kind: 'io' };
    if (LOGIC.has(word)) return { end, kind: 'logic' };
    if (TYPES.has(word)) return { end, kind: 'type' };
    if (ATOMS.has(word)) return { end, kind: 'atom' };
    if (METHODS.has(word) && line[pos - 1] === '.') return { end, kind: 'method' };
    if (/^\s*\(/.test(line.slice(end))) return { end, kind: 'function' };
    return { end, kind: 'variable' };
  }
  if ((m = rest.match(/^(?:==|!=|<=|>=|\+=|-=|\*\*|[+\-*/%<>=])/))) return { end: pos + m[0].length, kind: 'operator' };
  return { end: pos + 1, kind: 'plain' };
}

const escape = (s: string) => s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');

/** Código de Quimera como HTML con <span class="tk-…">; sirve para bloques de solo lectura. */
export function highlightToHtml(code: string): string {
  const state = startState();
  return code
    .replace(/\r\n/g, '\n')
    .split('\n')
    .map((line) => {
      let html = '';
      let pos = 0;
      while (pos < line.length) {
        const { end, kind } = scanToken(line, pos, state);
        const text = escape(line.slice(pos, end));
        html += kind === 'plain' || kind === 'variable' ? text : `<span class="tk-${kind}">${text}</span>`;
        pos = end;
      }
      return html;
    })
    .join('\n');
}
