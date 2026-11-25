// Resaltado de sintaxis de Quimera para CodeMirror (mismas palabras que el lexer en C)
import { HighlightStyle, StreamLanguage, syntaxHighlighting } from '@codemirror/language';
import { tags as t } from '@lezer/highlight';

const KEYWORDS = new Set([
  'If', 'ElseIf', 'Else', 'End', 'While', 'For', 'Return', 'Print', 'Read', 'And', 'Or', 'Not',
]);
const TYPES = new Set(['Integer', 'Float', 'String', 'Char', 'Boolean', 'List']);
const ATOMS = new Set(['True', 'False', 'Null']);

interface State {
  inComment: boolean;
}

const quimeraParser = {
  name: 'quimera',
  startState: (): State => ({ inComment: false }),
  token(stream: import('@codemirror/language').StringStream, state: State): string | null {
    if (state.inComment) {
      if (stream.skipTo('*/')) {
        stream.match('*/');
        state.inComment = false;
      } else {
        stream.skipToEnd();
      }
      return 'comment';
    }
    if (stream.eatSpace()) return null;
    if (stream.match('//')) {
      stream.skipToEnd();
      return 'comment';
    }
    if (stream.match('/*')) {
      state.inComment = true;
      return 'comment';
    }
    if (stream.match(/^"(?:[^"\\]|\\.)*"?/)) return 'string';
    if (stream.match(/^'(?:[^'\\]|\\.)'?/)) return 'string';
    if (stream.match(/^\d+(?:\.\d+)?/)) return 'number';
    if (stream.match(/^[A-Za-z_][A-Za-z0-9_]*/)) {
      const word = stream.current();
      if (KEYWORDS.has(word)) return 'keyword';
      if (TYPES.has(word)) return 'typeName';
      if (ATOMS.has(word)) return 'atom';
      if (stream.peek() === '(') return 'variableName.function';
      return 'variableName';
    }
    if (stream.match(/^(?:==|!=|<=|>=|\+=|-=|\*\*|[+\-*/%<>=.])/)) return 'operator';
    stream.next();
    return null;
  },
  languageData: { commentTokens: { line: '//', block: { open: '/*', close: '*/' } } },
};

export const quimeraLanguage = StreamLanguage.define(quimeraParser);

export const quimeraHighlight = syntaxHighlighting(
  HighlightStyle.define([
    { tag: t.keyword, color: 'var(--blue)', fontWeight: '500' },
    { tag: t.typeName, color: 'var(--green)' },
    { tag: t.atom, color: 'var(--red)' },
    { tag: t.string, color: 'var(--yellow)' },
    { tag: t.number, color: 'var(--red)' },
    { tag: t.comment, color: 'var(--muted)', fontStyle: 'italic' },
    { tag: t.function(t.variableName), color: 'var(--ink)', fontWeight: '500' },
    { tag: t.operator, color: 'var(--muted)' },
  ]),
);
