# Quimera — Compilador con IDE Integration

Compilador del lenguaje Quimera con sistema de diagnósticos unificado para integración en tiempo real con IDEs.

## ✨ Características Nuevas

- **Sistema de Diagnósticos JSON**: Salida estructurada en tiempo real
- **Modo Interactivo**: `--interactive` para IDEs
- **Comunicación Directa**: Sin threads, sin buffering
- **Códigos de Error Organizados**: Por etapa del compilador
- **Parser Unificado**: Solo interner, sin implementaciones mixtas

## 🚀 Uso Rápido

### Compilación Normal
```bash
# Con Makefile (recomendado)
make debug

# Con script personalizado
./build.bat
```

### Modo IDE (Interactivo)
```bash
./bin/Quimera.exe --interactive archivo.qmr
```

## 1) Requisitos e instalación

- Windows 11 x64 (Intel/AMD).
- MSYS2 (x86_64) instalado en `C:\msys64`.
- Toolchain GCC/GDB/Make.
- VS Code + extensión C/C++ (`ms-vscode.cpptools`).

**Pasos**

Instala MSYS2 (x86_64) y abre MSYS2 UCRT64 desde el Menú Inicio.

Actualiza:

```sh
pacman -Syu
# si lo pide, cierra y vuelve a abrir "MSYS2 UCRT64"
pacman -Syu
```

Instala toolchain:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain
```

Agrega al PATH (usuario): `C:\msys64\ucrt64\bin` y `C:\msys64\usr\bin`
Verifica en una terminal nueva:

```sh
gcc --version
gdb --version
make --version
where gcc
where gdb
where make
```

Si “no se reconoce” make usa `pacman -S --needed base-devel`.

---

## 2) Estructura del proyecto

```
quimera/
├─ include/            # headers (.h)
├─ src/                # código principal (.c)
│  ├─ main.c
│  ├─ lexer/…
│  ├─ parser/…
│  ├─ codegen/…
│  ├─ tables/…
│  ├─ utils/…
│  └─ apps/            # (opcional) ejecutables auxiliares *.c
├─ tests/              # cada test .c = 1 ejecutable
├─ obj/                # (autogenerado)
├─ bin/                # (autogenerado) app, tests y libs
└─ Makefile
```

---

## 3) Makefile (multi-ejecutables + tests)

El Makefile soporta:

- App principal: `bin/Quimera` (usa `src/main.c`).
- Librería estática con el core: `bin/libquimera.a` (sin main).
- Tests: cada `tests/*.c` → `bin/tests/<nombre>`.
- Apps opcionales: cada `src/apps/*.c` → `bin/apps/<nombre>`.

**Comandos**

- `make`            # = make debug (símbolos de depuración)
- `make debug`      # compila app + tests + apps (modo debug)
- `make release`    # binario optimizado (`-O2 -DNDEBUG`)
- `make run`        # compila (debug) y ejecuta `bin/Quimera`
- `make tests`      # compila todos los tests
- `make test`       # ejecuta todos los tests (falla ante el primero que falle)
- `make clean`      # borra bin/ y obj/

Añadir un test: crea `tests/mi_test.c`. Al compilar se generará `bin/tests/mi_test`.  
Añadir una app: crea `src/apps/mi_tool.c` → `bin/apps/mi_tool`.

---

## 4) VS Code (integración con Make + GDB)

Crea la carpeta `.vscode/` con estos archivos:

### `.vscode/tasks.json`

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Build (make, debug)",
      "type": "shell",
      "command": "make",
      "args": ["-j", "debug"],
      "problemMatcher": ["$gcc"],
      "group": { "kind": "build", "isDefault": true }
    },
    {
      "label": "Clean",
      "type": "shell",
      "command": "make",
      "args": ["clean"]
    }
  ]
}
```

### `.vscode/launch.json`

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug (gdb + make)",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/bin/Quimera",
      "cwd": "${workspaceFolder}",
      "MIMode": "gdb",
      "miDebuggerPath": "gdb",
      "externalConsole": true,
      "preLaunchTask": "Build (make, debug)"
    }
  ]
}
```

### `.vscode/c_cpp_properties.json`

```json
{
  "version": 4,
  "configurations": [
    {
      "name": "Windows (UCRT64)",
      "includePath": ["${workspaceFolder}/include"],
      "compilerPath": "C:/msys64/ucrt64/bin/gcc.exe",
      "cStandard": "c11",
      "intelliSenseMode": "windows-gcc-x64"
    }
  ]
}
```

**Uso en VS Code**

- Compilar: `Ctrl+Shift+B` → Build (make, debug)
- Depurar: `F5` → Debug (gdb + make)

Si ejecutas VS Code desde una terminal que ya tiene el PATH correcto (o tras reiniciar VS Code), GDB y GCC se detectan sin problemas.

---

## 5) Problemas comunes (y soluciones)

- **gcc/gdb no aparecen:** abre una terminal nueva tras editar PATH; verifica con `where gcc`. Asegura `C:\msys64\ucrt64\bin` primero en el PATH de usuario.
- **make no existe:** usa la terminal de MSYS2 en VS Code o agrega `C:\msys64\usr\bin` al PATH.
- **Errores con find desde CMD:** ejecuta make desde MSYS2 UCRT64 o Git Bash (o cambia a lista manual de fuentes en el Makefile).
- **Rutas con espacios:** entrecomilla rutas en `tasks.json`/`launch.json` o usa carpetas sin espacios.
- **Antivirus bloquea binarios:** marca el workspace como confiable o exclúyelo temporalmente.

---

## 6) Extensiones útiles (opcionales)

- **C/C++ (`ms-vscode.cpptools`)** — IntelliSense y depuración.
- **Clang-Format** — autoformato al guardar.
- **GitLens** — historial y blame.

---

## 7) Siguientes pasos

Añade tests por módulo (`tests/*.c`) y ejecútalos con `make test`.

Si quieres, agrega targets extra al Makefile: asan/ubsan, coverage, format, o CI (GitHub Actions).