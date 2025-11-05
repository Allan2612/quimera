#include "unity.h"
#include "diag_errors.h"

void register_lexer_tests(void);

void setUp(void)   { /* vacío a propósito */ }
void tearDown(void){ /* vacío a propósito */ }

int main(void) {
    // Inicializa una sola vez (modo síncrono; sin hilo)
    diag_init(false, NULL);

    UNITY_BEGIN();
    register_lexer_tests();
    int rc = UNITY_END();

    // Cierra una sola vez
    diag_shutdown();
    return rc;
}
