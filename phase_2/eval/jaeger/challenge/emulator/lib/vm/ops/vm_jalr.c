#include "vm_ops.h"

#include <stdint.h>

#include "r5/instruction.h"
#include "vm/vm.h"

enum result vm_jalr(struct vm * vm, const struct r5_instruction * ins) {
    enum result result;

    uint32_t rs1;
    
    if (    ((result = vm_get_register(vm, ins->rs1, &rs1)) != OK)   
         || ((result = vm_set_register(vm, ins->rd, vm->pc + 4)) != OK)) {
        return result;
    }

    vm->pc = rs1 + ins->immediate;

    return OK;
}