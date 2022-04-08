#include "vm_ops.h"

#include <stdint.h>

#include "r5/csr.h"
#include "r5/instruction.h"
#include "vm/vm.h"

enum result vm_mret(struct vm * vm, const struct r5_instruction * ins) {
    vm->pc = vm->mepc;

    switch (vm->mstatus.mpp) {
    case R5_MSTATUS_MPP_U: vm->level = VM_LEVEL_USER; break;
    case R5_MSTATUS_MPP_S: vm->level = VM_LEVEL_SUPERVISOR; break;
    case R5_MSTATUS_MPP_RESERVED: vm->level = VM_LEVEL_RESERVED; break;
    case R5_MSTATUS_MPP_M: vm->level = VM_LEVEL_MACHINE; break;
    }

    vm->mstatus.mie = vm->mstatus.mpie;

    return OK;
}