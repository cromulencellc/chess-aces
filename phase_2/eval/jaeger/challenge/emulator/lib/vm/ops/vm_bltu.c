#include "vm_ops.h"

#include <stdint.h>

#include "r5/instruction.h"
#include "vm/vm.h"

enum result vm_bltu(struct vm * vm, const struct r5_instruction * ins) {
    uint32_t lhs;
    uint32_t rhs;

    enum result result;

    if (    ((result = vm_get_register(vm, ins->rs1, &lhs)) != OK)
         || ((result = vm_get_register(vm, ins->rs2, &rhs)) != OK)) {
        return result;
    }

    if (lhs < rhs) {
        vm->pc += ins->immediate;
    }
    else {
        vm->pc += 4;
    }

    return OK;
}