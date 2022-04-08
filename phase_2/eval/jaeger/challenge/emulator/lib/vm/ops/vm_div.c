#include "vm_ops.h"

#include <stdint.h>

#include "r5/instruction.h"
#include "vm/vm.h"

enum result vm_div(struct vm * vm, const struct r5_instruction * ins) {
    int32_t lhs;
    int32_t rhs;

    enum result result;

    if (    ((result = vm_get_register(vm, ins->rs1, (uint32_t *) &lhs)) != OK)
         || ((result = vm_get_register(vm, ins->rs2, (uint32_t *) &rhs)) != OK)) {
        return result;
    }
    
    uint32_t rd;

    if (rhs == 0) {
        rd = 0;
    }
    else {
        rd = (uint32_t) (lhs / rhs);
    }
    
    if ((result = vm_set_register(vm, ins->rd, rd)) != OK) {
        return result;
    }

    vm->pc += 4;

    return OK;
}