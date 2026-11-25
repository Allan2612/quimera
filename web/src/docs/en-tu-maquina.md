---
description: Cómo compilar el intérprete de Quimera, ejecutar programas en la terminal, usar el IDE de escritorio y levantar este sitio en tu máquina.
---
# Usarlo en tu máquina

El código está en [github.com/Allan2612/quimera](https://github.com/Allan2612/quimera), con licencia MIT. Tiene tres partes: el intérprete en C, el IDE de escritorio y este sitio.

```sh
git clone https://github.com/Allan2612/quimera.git
cd quimera
```

## El intérprete en la terminal

Necesitas `gcc` y `make`. En Windows, instálalos con [MSYS2](https://www.msys2.org/) (`pacman -S make gcc`).

```sh
cd interprete
make
bin/Quimera programa.qmr
```

Los programas se guardan con extensión `.qmr`. La salida y los errores salen en JSON, una línea por mensaje (ver [Cómo funciona por dentro](/docs/por-dentro/#los-mensajes)). Si el programa usa `Read`, escribe los datos en la terminal o pásalos desde un archivo:

```sh
bin/Quimera programa.qmr < datos.txt
```

### Pruebas

```sh
make test            # pruebas unitarias del lexer, el parser, el analizador y el intérprete
make test-programs   # programas de ejemplo comparados con su salida esperada
```

## El IDE de escritorio

Un editor en Python con PyQt5: colores de sintaxis, números de línea, los errores marcados y la salida en un panel. Usa el intérprete compilado de `interprete/`.

```sh
cd ide
python -m venv .venv
.venv\Scripts\activate        # en Linux o macOS: source .venv/bin/activate
pip install PyQt5
python main.py
```

Se recomienda Python 3.12. El IDE escribe el programa en un archivo temporal y ejecuta el intérprete con `--interactive`, leyendo sus mensajes JSON.

## Este sitio

Necesitas Node 22.18 o más nuevo.

```sh
cd web
npm install
npm run dev      # sitio local con recarga automática
npm test         # corre los ejemplos, la documentación y las tarjetas con el intérprete en WebAssembly
npm run build    # versión final en web/dist
```

Si cambias el intérprete en C, vuelve a compilar el WebAssembly con `npm run build:wasm`. Usa `emcc` si lo tienes instalado y, si no, la imagen oficial de Emscripten en Docker.
