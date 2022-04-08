#include "vm_ops.h"

#include <stdint.h>

#include "r5/csr.h"
#include "r5/instruction.h"
#include "vm/vm.h"

enum result vm_ebreak(struct vm * vm, const struct r5_instruction * ins) {
    return vm_exception(vm, R5_CSR_MCAUSE_BREAKPOINT, 0);
}