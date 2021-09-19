#include "debug.h"

#include "intr.h"
#include "print.h"

void panic_spin(char* filename, int line, const char* func,
                const char* condition) {
    intr_disable();
    printf("%s:%d:%s:%s failed\n", filename, line, func, condition);
    while (1)
        ;
}
