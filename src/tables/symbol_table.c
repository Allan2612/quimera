#include "symbol_table.h"

// Reservadas
const char * const ReservedWords[] = {
    "Char","String","Integer","Float","Boolean","True","False","Null",
    "If","Else","For","While","Print"
};
const size_t ReservedWordCount = sizeof(ReservedWords)/sizeof(ReservedWords[0]);

// Tipos
const char * const BuiltinTypes[] = { "Char","String","Integer","Float","Boolean" };
const size_t BuiltinTypeCount = sizeof(BuiltinTypes)/sizeof(BuiltinTypes[0]);

// Delimitadores (mejor tokens individuales para el futuro lexer)
const char * const Delimiters[] = { ";", ":", "{", "}", ",", "(", ")", "[", "]" };
const size_t DelimiterCount = sizeof(Delimiters)/sizeof(Delimiters[0]);

// Aritméticos
const char * const ArithmeticOps[] = { "+","-","*","/","%","**" };
const size_t ArithmeticOpCount = sizeof(ArithmeticOps)/sizeof(ArithmeticOps[0]);

// Lógicos / comparación
const char * const LogicCompareOps[] = { "==","!=",">","<",">=","<=","&&","||","!" };
const size_t LogicCompareOpCount = sizeof(LogicCompareOps)/sizeof(LogicCompareOps[0]);

// Asignación
const char * const AssignmentOps[] = { "=", "+=","-="};
const size_t AssignmentOpCount = sizeof(AssignmentOps)/sizeof(AssignmentOps[0]);

// ===== Comentarios =====
const char * const LineCommentStart  = "//";
const char * const BlockCommentStart = "/*";
const char * const BlockCommentEnd   = "*/";
