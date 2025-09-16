#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" { 
#endif

// Palabras reservadas
extern const char * const ReservedWords[];
extern const size_t       ReservedWordCount;

// Tipos incorporados
extern const char * const BuiltinTypes[];
extern const size_t       BuiltinTypeCount;

// Delimitadores  (recomendado separar tokens simples: { } ( ) [ ] ; : , )
extern const char * const Delimiters[];
extern const size_t       DelimiterCount;

// Operadores aritméticos
extern const char * const ArithmeticOps[];
extern const size_t       ArithmeticOpCount;

// Operadores lógicos/comp.
extern const char * const LogicCompareOps[];
extern const size_t       LogicCompareOpCount;

#ifdef __cplusplus
}
#endif
#endif
