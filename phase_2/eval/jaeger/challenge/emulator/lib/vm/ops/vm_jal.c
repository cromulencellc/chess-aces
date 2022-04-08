#include "vm_ops.h"

#include <stdint.h>

#include "r5/instruction.h"
#include "vm/vm.h"

enum result vm_jal(struct vm * vm, const struct r5_instruction * ins) {
    enum result result;
    
    if ((result = vm_set_register(vm, ins->rd, vm->pc + 4)) != OK) {
        return result;
    }

    vm->pc += ins->immediate;

    return OK;
}