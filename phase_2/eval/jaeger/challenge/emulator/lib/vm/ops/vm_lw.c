#include "vm_ops.h"

#include <stdint.h>

#include "r5/csr.h"
#include "r5/instruction.h"
#include "vm/vm.h"
#include "vm/vm_mem.h"

enum result vm_lw(struct vm * vm, const struct r5_instruction * ins) {
    uint32_t address;

    enum result result;

    if ((result = vm_get_register(vm, ins->rs1, &address)) != OK) {
        return result;
    }

    address += ins->immediate;

    uint32_t value;

    if (vm_get_u32(vm, address, &value)) {
        return vm_exception(vm, R5_CSR_MCAUSE_LOAD_PAGE_FAULT, address);
    }
    
    if ((result = vm_set_register(vm, ins->rd, value)) != OK) {
        return result;
    }

    vm->pc += 4;

    return OK;
}