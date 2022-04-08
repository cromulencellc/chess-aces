#include "vm_ops.h"

#include <stdint.h>

#include "r5/instruction.h"
#include "vm/vm.h"

enum result vm_slt(struct vm * vm, const struct r5_instruction * ins) {
    uint32_t lhs;
    uint32_t rhs;

    enum result result;

    if (    ((result = vm_get_register(vm, ins->rs1, &lhs)) != OK)
         || ((result = vm_get_register(vm, ins->rs2, &rhs)) != OK)) {
        return result;
    }

    if ((int32_t) lhs < (int32_t) rhs) {
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