#include "vm_ops.h"

#include <stdint.h>
#include <stdio.h>

#include "r5/instruction.h"
#include "vm/vm.h"

enum result vm_srai(struct vm * vm, const struct r5_instruction * ins) {
    uint32_t lhs;

    enum result result;
    if ((result = vm_get_register(vm, ins->rs1, &lhs)) != OK) {
        return result;
    }

    uint32_t shifted = lhs >> ins->immediate;
    if (lhs & 0x80000000) {
        shifted |= 0xffffffff << ins->immediate;
    }

    // printf("srai 0x%08x 0x%08x 0x%08x\n", lhs, ins->immediate, shifted);

    if ((result = vm_set_register(vm, ins->rd, shifted)) != OK) {
        return result;
    }

    vm->pc += 4;

    return OK;
}