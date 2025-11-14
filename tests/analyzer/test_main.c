#include "unity.h"
#include "test_runner.h"
#include <stdio.h>

// Función setUp requerida por Unity - se ejecuta antes de cada test
void setUp(void) {
    // Inicialización antes de cada test (actualmente no necesaria)
}

// Función tearDown requerida por Unity - se ejecuta después de cada test  
void tearDown(void) {
    // Limpieza después de cada test (actualmente no necesaria)
}

// Función principal para ejecutar todos los tests del analizador semántico
int main(void) {
    printf("=== Ejecutando Tests del Analizador Semántico de Quimera ===\n\n");
    
    int result = run_all_analyzer_tests();
    
    if (result == 0) {
        printf("\n✅ Todos los tests del analizador semántico pasaron exitosamente\n");
    } else {
        printf("\n❌ Algunos tests del analizador semántico fallaron\n");
    }
    
    return result;
}