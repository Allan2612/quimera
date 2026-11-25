import { resolve } from 'node:path';
import { defineConfig } from 'vite';
import { DOCS, docsPlugin } from './docs-plugin.ts';

// Varias páginas: la portada y una por documento (web/docs/**/index.html)
const pages = Object.fromEntries([
  ['main', resolve(import.meta.dirname, 'index.html')],
  ['aprende', resolve(import.meta.dirname, 'aprende/index.html')],
  ['retos', resolve(import.meta.dirname, 'retos/index.html')],
  ...DOCS.map((d) => [`docs-${d.slug}`, resolve(import.meta.dirname, `.${d.path}index.html`)]),
]);

export default defineConfig({
  plugins: [docsPlugin()],
  worker: { format: 'es' },
  build: { target: 'es2022', rollupOptions: { input: pages } },
});
