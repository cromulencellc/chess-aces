#include "rust.h"

#include <stdio.h>
#include <stdlib.h>

int try_err;


void unimplemented() {
    printf("UNIMPLEMENTED\n");
    fflush(stdout);
    exit(-1);
}

void * unwrap_null_(void * p, const char * function, unsigned int line) {
    if (p == NULL) {
        printf("[NULL UNWRAP] %s:%u\n", function, line);
        fflush(stdout);
        exit(-1);
    }
    return p;
}