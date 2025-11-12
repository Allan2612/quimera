#include "unity.h"
#include "diag.h"

void register_ast_tests(void);

void setUp(void)   { /* vacío a propósito */ }
void tearDown(void){ /* vacío a propósito */ }

int main(void) {
    // Inicializa una sola vez (modo síncrono; sin hilo)
    diag_init();

    UNITY_BEGIN();
    register_ast_tests();
    int rc = UNITY_END();

    // Cierra una sola vez
    diag_shutdown();
    return rc;
}
