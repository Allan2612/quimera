@echo off
echo Compilando proyecto Quimera...

REM Crear directorios
mkdir obj\src\ast 2>nul
mkdir obj\src\codegen 2>nul
mkdir obj\src\interner 2>nul
mkdir obj\src\interpreter 2>nul
mkdir obj\src\lexer 2>nul
mkdir obj\src\parser 2>nul
mkdir obj\src\tables 2>nul
mkdir obj\src\utils 2>nul
mkdir bin 2>nul

REM Flags de compilación
set CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -g -O0 -I include

echo Compilando archivos fuente...
gcc %CFLAGS% -c src\ast\ast.c -o obj\src\ast\ast.o
gcc %CFLAGS% -c src\codegen\codegen.c -o obj\src\codegen\codegen.o
gcc %CFLAGS% -c src\interner\interner.c -o obj\src\interner\interner.o
gcc %CFLAGS% -c src\interpreter\interpreter.c -o obj\src\interpreter\interpreter.o
gcc %CFLAGS% -c src\lexer\lexer.c -o obj\src\lexer\lexer.o
gcc %CFLAGS% -c src\parser\parser.c -o obj\src\parser\parser.o
gcc %CFLAGS% -c src\tables\symbol_table.c -o obj\src\tables\symbol_table.o
gcc %CFLAGS% -c src\utils\diag.c -o obj\src\utils\diag.o
gcc %CFLAGS% -c src\utils\utils.c -o obj\src\utils\utils.o

echo Creando biblioteca estática...
ar rcs bin\libquimera.a obj\src\ast\ast.o obj\src\codegen\codegen.o obj\src\interner\interner.o obj\src\interpreter\interpreter.o obj\src\lexer\lexer.o obj\src\parser\parser.o obj\src\tables\symbol_table.o obj\src\utils\diag.o obj\src\utils\utils.o

echo Compilando main.c...
gcc %CFLAGS% -c src\main.c -o obj\src\main.o

echo Linkeando ejecutable...
gcc %CFLAGS% obj\src\main.o bin\libquimera.a -o bin\Quimera.exe

if exist bin\Quimera.exe (
    echo ¡Compilación exitosa! Ejecutable en: bin\Quimera.exe
) else (
    echo Error en la compilación
    exit /b 1
)