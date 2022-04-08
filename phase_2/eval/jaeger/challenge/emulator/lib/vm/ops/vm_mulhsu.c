#include "vm_ops.h"

#include <stdint.h>

#include "r5/instruction.h"
#include "vm/vm.h"

enum result vm_mulhsu(struct vm * vm, const struct r5_instruction * ins) {
    uint64_t lhs;
    uint64_t rhs;

    uint32_t lhs32;
    uint32_t rhs32;

    enum result result;

    if (    ((result = vm_get_register(vm, ins->rs1, &lhs32)) != OK)
         || ((result = vm_get_register(vm, ins->rs2, &rhs32)) != OK)) {
        return result;
    }

    lhs = lhs32;
    rhs = rhs32;

    if (lhs & 0x80000000) {
        lhs |= 0xffffffff00000000;
    }

    uint64_t rd = lhs * rhs;
    rd = rd >> 32;

    if ((result = vm_set_register(vm, ins->rd, rd)) != OK) {
        return result;
    }

    vm->pc += 4;

    return OK;
}