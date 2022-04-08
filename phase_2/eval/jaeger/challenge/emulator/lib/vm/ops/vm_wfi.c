#include "vm_ops.h"

#include <stdint.h>

#include "r5/csr.h"
#include "r5/instruction.h"
#include "vm/vm.h"

enum result vm_wfi(struct vm * vm, const struct r5_instruction * ins) {
    vm->wfi = 1;

    return OK;
}