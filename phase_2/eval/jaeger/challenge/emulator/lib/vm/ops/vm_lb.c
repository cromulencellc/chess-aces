#include "vm_ops.h"

#include <stdint.h>

#include "r5/csr.h"
#include "r5/instruction.h"
#include "vm/vm.h"
#include "vm/vm_mem.h"

enum result vm_lb(struct vm * vm, const struct r5_instruction * ins) {
    uint32_t address;

    enum result result;

    if ((result = vm_get_register(vm, ins->rs1, &address)) != OK) {
        return result;
    }

    address += ins->immediate;

    uint8_t value;

    if (vm_get_u8(vm, address, &value)) {
        return vm_exception(vm, R5_CSR_MCAUSE_LOAD_PAGE_FAULT, address);
    }

    uint32_t value32 = value;
    if (value32 & 0x80) {
        value32 |= 0xffffff00;
    }
    
    if ((result = vm_set_register(vm, ins->rd, value32)) != OK) {
        return result;
    }

    vm->pc += 4;

    return OK;
}