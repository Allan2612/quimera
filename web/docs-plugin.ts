// Documentación del sitio: cada web/docs/**/index.html trae solo un marcador <!--doc:slug-->. Este
// plugin lo cambia por la página completa, a partir de src/docs/<slug>.md, con el encabezado y el pie
// del sitio. El código de Quimera sale ya coloreado (sin JavaScript) y con un enlace que lo abre en el
// editor de la portada.
import { readFileSync } from 'node:fs';
import { resolve } from 'node:path';
import { Marked, type Tokens } from 'marked';
import type { Plugin } from 'vite';
import { highlightToHtml } from './src/quimera-tokens.ts';

export const DOCS = [
  { slug: 'inicio', path: '/docs/', title: 'Inicio rápido' },
  { slug: 'referencia', path: '/docs/referencia/', title: 'Referencia del lenguaje' },
  { slug: 'errores', path: '/docs/errores/', title: 'Errores' },
  { slug: 'por-dentro', path: '/docs/por-dentro/', title: 'Cómo funciona por dentro' },
  { slug: 'en-tu-maquina', path: '/docs/en-tu-maquina/', title: 'Usarlo en tu máquina' },
];

const SITE = 'https://quimera.avt.codes';
const escape = (s: string) => s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
const slugify = (s: string) =>
  s.toLowerCase().normalize('NFD').replace(/[\u0300-\u036f]/g, '').replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');

/**
 * Atributos del bloque de código: ```qmr error=3000 input=Ana María|21
 * `input` va al final y toma el resto de la línea (puede tener espacios); `|` separa las líneas de entrada.
 */
export function fenceInfo(info: string) {
  const [head, input = ''] = info.trim().split(/\s+input=/);
  const [lang = '', ...rest] = head.split(/\s+/);
  const attrs = Object.fromEntries(rest.map((kv) => kv.split('=') as [string, string]));
  return { lang, error: attrs.error as string | undefined, input: input.replace(/\|/g, '\n') };
}

/** El mismo formato que el enlace "Copiar enlace" de la portada (main.ts) */
const shareHash = (code: string, input: string) => Buffer.from(JSON.stringify({ code, input })).toString('base64url');

function render(md: string) {
  const toc: { id: string; text: string }[] = [];
  const marked = new Marked({
    renderer: {
      code({ text, lang }: Tokens.Code) {
        const info = fenceInfo(lang ?? '');
        if (info.lang !== 'qmr') return `<pre class="code"><code>${escape(text)}</code></pre>\n`;
        const open = `<a class="btn-ghost btn-small open-editor" href="/#p=${shareHash(text + '\n', info.input)}">Abrir en el editor</a>`;
        const mark = info.error ? ` data-error="${info.error}"` : '';
        return `<div class="code-block"${mark}><pre><code class="qmr" data-hl>${highlightToHtml(text)}</code></pre>${open}</div>\n`;
      },
      heading({ tokens, depth }: Tokens.Heading) {
        const html = this.parser.parseInline(tokens);
        const plain = html.replace(/<[^>]+>/g, '');
        const code = plain.match(/^(\d{4})\b/);
        const id = code ? `e${code[1]}` : slugify(plain);
        if (depth === 2) toc.push({ id, text: plain });
        return `<h${depth} id="${id}">${html}</h${depth}>\n`;
      },
    },
  });
  const body = marked.parse(md, { async: false }) as string;
  return { body, toc };
}

function page(slug: string, root: string): string {
  const i = DOCS.findIndex((d) => d.slug === slug);
  if (i < 0) throw new Error(`Documento desconocido: ${slug}`);
  const doc = DOCS[i];
  const raw = readFileSync(resolve(root, 'src/docs', `${slug}.md`), 'utf8').replace(/\r\n/g, '\n');
  const front = raw.match(/^---\ndescription: (.+)\n---\n/);
  const description = front?.[1] ?? '';
  const { body, toc } = render(front ? raw.slice(front[0].length) : raw);
  const prev = DOCS[i - 1];
  const next = DOCS[i + 1];
  const title = `${doc.title} · Documentación de Quimera`;
  const nav = DOCS.map((d) => `<li><a href="${d.path}"${d.slug === slug ? ' aria-current="page"' : ''}>${d.title}</a></li>`).join('');
  const onPage = toc.length > 1 ? `<p class="docs-nav-label">En esta página</p><ul class="docs-toc">${toc.map((t) => `<li><a href="#${t.id}">${escape(t.text)}</a></li>`).join('')}</ul>` : '';

  return `<!doctype html>
<html lang="es">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <script>
      (function () {
        var d = document.documentElement, t;
        d.classList.add('js');
        try { t = localStorage.getItem('quimera.tema'); } catch (e) {}
        var dark = t === 'oscuro' || (t !== 'claro' && matchMedia('(prefers-color-scheme: dark)').matches);
        d.dataset.theme = dark ? 'dark' : 'light';
      })();
    </script>
    <title>${escape(title)}</title>
    <meta name="description" content="${escape(description)}" />
    <link rel="canonical" href="${SITE}${doc.path}" />
    <meta property="og:title" content="${escape(title)}" />
    <meta property="og:description" content="${escape(description)}" />
    <meta property="og:url" content="${SITE}${doc.path}" />
    <meta property="og:type" content="article" />
    <meta name="theme-color" content="#FBFBFA" media="(prefers-color-scheme: light)" />
    <meta name="theme-color" content="#161615" media="(prefers-color-scheme: dark)" />
    <link rel="icon" href="/favicon.svg" type="image/svg+xml" />
    <link rel="preconnect" href="https://fonts.googleapis.com" />
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
    <link href="https://fonts.googleapis.com/css2?family=Geist:wght@400;500;600&family=Geist+Mono:wght@400;500&family=Instrument+Serif:ital@0;1&display=swap" rel="stylesheet" />
  </head>
  <body>
    <div class="ambient" aria-hidden="true"></div>
    <header class="topbar">
      <a class="wordmark" href="/">Quimera</a>
      <div class="topbar-end">
        <nav aria-label="Secciones">
          <a href="/#probar">Probar</a>
          <a href="/docs/">Documentación</a>
          <a href="https://github.com/Allan2612/quimera" rel="noopener">GitHub</a>
        </nav>
        <button id="theme" class="icon-btn" type="button" aria-label="Cambiar tema"></button>
      </div>
    </header>
    <div class="docs">
      <aside class="docs-nav" aria-label="Documentación">
        <ul>${nav}</ul>
        ${onPage}
      </aside>
      <main class="doc">
        ${body}
        <nav class="doc-pager" aria-label="Páginas">
          ${prev ? `<a href="${prev.path}"><small>Anterior</small>${prev.title}</a>` : '<span></span>'}
          ${next ? `<a class="next" href="${next.path}"><small>Siguiente</small>${next.title}</a>` : '<span></span>'}
        </nav>
      </main>
    </div>
    <footer class="footer">
      <p>Hecho por <strong>Raúl Bejarano Mora</strong> y <strong>Allan Vargas Torres</strong> para el curso EIF400 Paradigmas de Programación de la Universidad Nacional de Costa Rica.</p>
      <p class="muted">Código abierto con licencia MIT en <a href="https://github.com/Allan2612/quimera" rel="noopener">GitHub</a>.</p>
    </footer>
    <script type="module" src="/src/docs-page.ts"></script>
  </body>
</html>
`;
}

export function docsPlugin(): Plugin {
  let root = process.cwd();
  return {
    name: 'quimera-docs',
    configResolved(config) {
      root = config.root;
    },
    // Los .md también cuentan como fuente: al editarlos, Vite recarga la página
    configureServer(server) {
      server.watcher.add(resolve(root, 'src/docs'));
      server.watcher.on('change', (file) => {
        if (file.endsWith('.md')) server.ws.send({ type: 'full-reload' });
      });
    },
    transformIndexHtml: {
      order: 'pre',
      handler(html) {
        const m = html.match(/<!--doc:([a-z-]+)-->/);
        return m ? page(m[1], root) : html;
      },
    },
  };
}
