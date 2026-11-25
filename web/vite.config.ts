import { resolve } from 'node:path';
import { defineConfig } from 'vite';
import { DOCS, docsPlugin } from './docs-plugin';

// Varias páginas: la portada y una por documento (web/docs/**/index.html)
const pages = Object.fromEntries([
  ['main', resolve(__dirname, 'index.html')],
  ['aprende', resolve(__dirname, 'aprende/index.html')],
  ['retos', resolve(__dirname, 'retos/index.html')],
  ...DOCS.map((d) => [`docs-${d.slug}`, resolve(__dirname, `.${d.path}index.html`)]),
]);

export default defineConfig({
  plugins: [docsPlugin()],
  worker: { format: 'es' },
  build: { target: 'es2022', rollupOptions: { input: pages } },
});
