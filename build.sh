#!/bin/bash

# Script para compilar el proyecto Quimera manualmente debido a problemas con Makefile en Windows

echo "Compilando proyecto Quimera..."

# Variables
SRC_DIR="src"
OBJ_DIR="obj"  
BIN_DIR="bin"
INCLUDE_DIR="include"

# Limpiar objetos anteriores
rm -f obj/src/*.o obj/src/*/*.o bin/libquimera.a bin/Quimera.exe 2>/dev/null

# Crear directorios necesarios
mkdir -p "$OBJ_DIR/src/ast"
mkdir -p "$OBJ_DIR/src/codegen"
mkdir -p "$OBJ_DIR/src/interner"
mkdir -p "$OBJ_DIR/src/interpreter"
mkdir -p "$OBJ_DIR/src/lexer"
mkdir -p "$OBJ_DIR/src/parser"
mkdir -p "$OBJ_DIR/src/tables"
mkdir -p "$OBJ_DIR/src/utils"
mkdir -p "$BIN_DIR"

# Flags de compilación
CFLAGS="-std=c11 -Wall -Wextra -Wpedantic -g -O0 -I$INCLUDE_DIR -MMD -MP"

# Compilar todos los archivos fuente (excepto main.c)
echo "Compilando archivos fuente..."

gcc $CFLAGS -c src/ast/ast.c -o obj/src/ast/ast.o
gcc $CFLAGS -c src/codegen/codegen.c -o obj/src/codegen/codegen.o
gcc $CFLAGS -c src/interner/interner.c -o obj/src/interner/interner.o
gcc $CFLAGS -c src/interpreter/interpreter.c -o obj/src/interpreter/interpreter.o
gcc $CFLAGS -c src/lexer/lexer.c -o obj/src/lexer/lexer.o
gcc $CFLAGS -c src/parser/parser.c -o obj/src/parser/parser.o
gcc $CFLAGS -c src/tables/symbol_table.c -o obj/src/tables/symbol_table.o
gcc $CFLAGS -c src/utils/diag.c -o obj/src/utils/diag.o
gcc $CFLAGS -c src/utils/utils.c -o obj/src/utils/utils.o

# Crear biblioteca estática
echo "Creando biblioteca estática..."
ar rcs bin/libquimera.a \
    obj/src/ast/ast.o \
    obj/src/codegen/codegen.o \
    obj/src/interner/interner.o \
    obj/src/interpreter/interpreter.o \
    obj/src/lexer/lexer.o \
    obj/src/parser/parser.o \
    obj/src/tables/symbol_table.o \
    obj/src/utils/diag.o \
    obj/src/utils/utils.o

# Compilar main.c
echo "Compilando main.c..."
gcc $CFLAGS -c src/main.c -o obj/src/main.o

# Linkear ejecutable final
echo "Linkeando ejecutable..."
gcc $CFLAGS obj/src/main.o bin/libquimera.a -o bin/Quimera.exe

if [ -f bin/Quimera.exe ]; then
    echo "¡Compilación exitosa! Ejecutable en: bin/Quimera.exe"
else
    echo "Error en la compilación"
    exit 1
fi