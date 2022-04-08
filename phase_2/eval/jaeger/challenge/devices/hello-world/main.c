#include "platform/platform.h"

void interrupt_handler() {}


void main () {
    set_interrupt_handler(&interrupt_handler);
    char * stdout = (char *) 0x80000000;

    char * s = "Hello World\n";

    while (*s) {
        *stdout = *s;
        s++;
    }
}