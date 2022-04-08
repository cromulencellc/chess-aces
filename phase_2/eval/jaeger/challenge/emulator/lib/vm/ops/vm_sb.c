#include "vm_ops.h"

#include <stdint.h>

#include "r5/csr.h"
#include "r5/instruction.h"
#include "vm/vm.h"
#include "vm/vm_mem.h"

enum result vm_sb(struct vm * vm, const struct r5_instruction * ins) {
    uint32_t rs1;
    uint32_t rs2;
    uint32_t address;

    enum result result;

    if (    ((result = vm_get_register(vm, ins->rs1, &rs1)) != OK)
         || ((result = vm_get_register(vm, ins->rs2, &rs2)) != OK)) {
        return result;
    }

    address = rs1 + ins->immediate;

    if (vm_set_u8(vm, address, rs2)) {
        return vm_exception(vm, R5_CSR_MCAUSE_STORE_AMO_PAGE_FAULT, address);
    }

    vm->pc += 4;

    return OK;
}