#include "unity.h"
#include "diag_errors.h"

void register_all_tests(void);

int main(void) {
    // Inicializa una sola vez (modo síncrono; sin hilo)
    diag_init(false, NULL);

    UNITY_BEGIN();
    register_all_tests();
    int rc = UNITY_END();

    // Cierra una sola vez
    diag_shutdown();
    return rc;
}
