#include "vm_ops.h"

#include <stdint.h>

#include "r5/instruction.h"
#include "vm/vm.h"

enum result vm_slti(struct vm * vm, const struct r5_instruction * ins) {
    uint32_t lhs;

    enum result result;

    if ((result = vm_get_register(vm, ins->rs1, &lhs)) != OK) {
        return result;
    }

    if ((int32_t) lhs < (int32_t) ins->immediate) {
        if ((result = vm_set_register(vm, ins->rd, 1)) != 0) {
            return result;
        }
    }
    else {
        if ((result = vm_set_register(vm, ins->rd, 0)) != 0) {
            return result;
        }
    }

    vm->pc += 4;

    return OK;
}