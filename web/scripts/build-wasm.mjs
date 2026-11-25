// Compila el intérprete en C (../interprete) a WebAssembly: src/wasm/quimera.mjs + quimera.wasm.
// Usa `emcc` si está en el PATH (CI) y, si no, la imagen oficial de Emscripten en Docker.
import { execFileSync, spawnSync } from 'node:child_process';
import { mkdirSync, readdirSync, statSync } from 'node:fs';
import { dirname, join, relative, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const EMSDK_IMAGE = 'emscripten/emsdk:3.1.74';
const web = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const repo = resolve(web, '..');
const interp = join(repo, 'interprete');

function cFiles(dir) {
  return readdirSync(dir).flatMap((name) => {
    const p = join(dir, name);
    if (statSync(p).isDirectory()) return cFiles(p);
    return p.endsWith('.c') ? [p] : [];
  });
}

const sources = cFiles(join(interp, 'src')).map((p) => relative(repo, p).replaceAll('\\', '/')).sort();
const flags = [
  '-std=c11', '-O2', '-I', 'interprete/include',
  ...sources,
  '-o', 'web/src/wasm/quimera.mjs',
  '-sMODULARIZE=1', '-sEXPORT_ES6=1',
  '-sEXPORTED_RUNTIME_METHODS=FS,callMain',
  '-sINVOKE_RUN=0', '-sEXIT_RUNTIME=1',
  '-sALLOW_MEMORY_GROWTH=1',
  // Pila amplia: la recursión de Quimera usa varias funciones de C por llamada (límite: 400)
  '-sSTACK_SIZE=4MB',
  '-sENVIRONMENT=web,worker,node',
];

mkdirSync(join(web, 'src', 'wasm'), { recursive: true });

const hasEmcc = spawnSync('emcc', ['--version']).status === 0;
if (hasEmcc) {
  execFileSync('emcc', flags, { cwd: repo, stdio: 'inherit' });
} else {
  execFileSync('docker', ['run', '--rm', '-v', `${repo}:/src`, '-w', '/src', EMSDK_IMAGE, 'emcc', ...flags], {
    stdio: 'inherit',
    env: { ...process.env, MSYS_NO_PATHCONV: '1' },
  });
}
console.log('WebAssembly listo en web/src/wasm/');
