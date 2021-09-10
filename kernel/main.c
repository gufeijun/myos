#include "print.h"

int main() {
    int i;
    char buf[10] = "hello";
    for (i = 0; i < 1023; i++) {
        buf[5] = 0;
        put_str(buf);
    }
    while (1)
        ;
    return 0;
}
