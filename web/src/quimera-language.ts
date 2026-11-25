// Quimera en CodeMirror: usa el mismo tokenizador que los bloques de código de la página
// (quimera-tokens.ts) y pinta con las mismas clases .tk-* (colores en styles.css).
import { HighlightStyle, StreamLanguage, syntaxHighlighting, type StringStream } from '@codemirror/language';
import { tags as t } from '@lezer/highlight';
import { scanToken, startState, type ScanState, type TokenKind } from './quimera-tokens';

/* Tipo de token -> nombre de estilo de CodeMirror (cada uno corresponde a una etiqueta de @lezer/highlight) */
const STYLE: Record<TokenKind, string | null> = {
  control: 'controlKeyword',
  io: 'keyword',
  logic: 'operatorKeyword',
  type: 'typeName',
  atom: 'atom',
  string: 'string',
  number: 'number',
  comment: 'comment',
  method: 'propertyName',
  function: 'variableName.function',
  variable: 'variableName',
  operator: 'operator',
  plain: null,
};

export const quimeraLanguage = StreamLanguage.define<ScanState>({
  name: 'quimera',
  startState,
  copyState: (s) => ({ ...s }),
  token(stream: StringStream, state: ScanState) {
    const { end, kind } = scanToken(stream.string, stream.pos, state);
    stream.pos = Math.max(end, stream.pos + 1);
    return STYLE[kind];
  },
  languageData: { commentTokens: { line: '//', block: { open: '/*', close: '*/' } } },
});

export const quimeraHighlight = syntaxHighlighting(
  HighlightStyle.define([
    { tag: t.controlKeyword, class: 'tk-control' },
    { tag: t.keyword, class: 'tk-io' },
    { tag: t.operatorKeyword, class: 'tk-logic' },
    { tag: t.typeName, class: 'tk-type' },
    { tag: t.atom, class: 'tk-atom' },
    { tag: t.string, class: 'tk-string' },
    { tag: t.number, class: 'tk-number' },
    { tag: t.comment, class: 'tk-comment' },
    { tag: t.propertyName, class: 'tk-method' },
    { tag: t.function(t.variableName), class: 'tk-function' },
    { tag: t.operator, class: 'tk-operator' },
  ]),
);
