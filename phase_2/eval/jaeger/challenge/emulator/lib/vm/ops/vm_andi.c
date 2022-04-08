#include "vm_ops.h"

#include <stdint.h>

#include "r5/instruction.h"
#include "vm/vm.h"

enum result vm_andi(struct vm * vm, const struct r5_instruction * ins) {
    uint32_t lhs;

    enum result result;

    if (    ((result = vm_get_register(vm, ins->rs1, &lhs)) != OK)
         || ((result = vm_set_register(vm,
                                       ins->rd,
                                       lhs & ins->immediate)) != OK)) {
        return result;
    }

    vm->pc += 4;

    return OK;
}