#include "vm/csr.h"

#include <stdlib.h>

#include "error.h"
#include "r5/csr.h"
#include "vm.h"

uint32_t vm_csr_misa_read(struct vm * vm) {
    return R5_CSR_MISA_VALUE;
}

int vm_csr_misa_write(struct vm * vm, uint32_t value) {
    return OK;
}

uint32_t vm_csr_mvendorid_read(struct vm * vm) {
    return R5_CSR_MVENDORID_VALUE;
}
int vm_csr_mvendorid_write(struct vm * vm, uint32_t value) {
    return OK;
}

uint32_t vm_csr_marchid_read(struct vm * vm) {
    return R5_CSR_MARCHID_VALUE;
}
int vm_csr_marchid_write(struct vm * vm, uint32_t value) {
    return OK;
}

uint32_t vm_csr_mimpid_read(struct vm * vm) {
    return R5_CSR_MIMPID_VALUE;
}
int vm_csr_mimpid_write(struct vm * vm, uint32_t value) {
    return OK;
}

uint32_t vm_csr_mhartid_read(struct vm * vm) {
    return R5_CSR_MHARTID_VALUE;
}
int vm_csr_mhartid_write(struct vm * vm, uint32_t value) {
    return OK;
}

uint32_t vm_csr_mstatus_read(struct vm * vm) {
    uint32_t mstatus = 0;
    if (vm->mstatus.uie) { mstatus |= R5_CSR_MSTATUS_UIE; }
    if (vm->mstatus.sie) { mstatus |= R5_CSR_MSTATUS_UIE; }
    if (vm->mstatus.mie) { mstatus |= R5_CSR_MSTATUS_UIE; }
    if (vm->mstatus.upie) { mstatus |= R5_CSR_MSTATUS_UIE; }
    if (vm->mstatus.spie) { mstatus |= R5_CSR_MSTATUS_UIE; }
    if (vm->mstatus.mpie) { mstatus |= R5_CSR_MSTATUS_UIE; }
    if (vm->mstatus.spp) { mstatus |= R5_CSR_MSTATUS_SPP; }

    switch (vm->mstatus.mpp) {
    case R5_MSTATUS_MPP_U: mstatus |= R5_CSR_MSTATUS_MPP_U; break;
    case R5_MSTATUS_MPP_S: mstatus |= R5_CSR_MSTATUS_MPP_S; break;
    case R5_MSTATUS_MPP_RESERVED: break;
    case R5_MSTATUS_MPP_M: mstatus |= R5_CSR_MSTATUS_MPP_M; break;
    }

    switch (vm->mstatus.fs) {
    case R5_MSTATUS_FS_OFF: mstatus |= R5_CSR_MSTATUS_FS_OFF; break;
    case R5_MSTATUS_FS_INITIAL: mstatus |= R5_CSR_MSTATUS_FS_INITIAL; break;
    case R5_MSTATUS_FS_CLEAN: mstatus |= R5_CSR_MSTATUS_FS_CLEAN; break;
    case R5_MSTATUS_FS_DIRTY: mstatus |= R5_CSR_MSTATUS_FS_DIRTY; break;
    }

    switch (vm->mstatus.xs) {
    case R5_MSTATUS_XS_OFF: mstatus |= R5_CSR_MSTATUS_XS_OFF; break;
    case R5_MSTATUS_XS_INITIAL: mstatus |= R5_CSR_MSTATUS_XS_INITIAL; break;
    case R5_MSTATUS_XS_CLEAN: mstatus |= R5_CSR_MSTATUS_XS_CLEAN; break;
    case R5_MSTATUS_XS_DIRTY: mstatus |= R5_CSR_MSTATUS_XS_DIRTY; break;
    }

    if (vm->mstatus.mprv) { mstatus |= R5_CSR_MSTATUS_MPRV; }
    if (vm->mstatus.sum) { mstatus |= R5_CSR_MSTATUS_SUM; }
    if (vm->mstatus.mxr) { mstatus |= R5_CSR_MSTATUS_MXR; }
    if (vm->mstatus.tvm) { mstatus |= R5_CSR_MSTATUS_TVM; }
    if (vm->mstatus.tw) { mstatus |= R5_CSR_MSTATUS_TW; }
    if (vm->mstatus.tsr) { mstatus |= R5_CSR_MSTATUS_TSR; }
    if (vm->mstatus.sd) { mstatus |= R5_CSR_MSTATUS_SD; }

    return mstatus;
}

int vm_csr_mstatus_write(struct vm * vm, uint32_t value) {
    vm->mstatus.uie = value & R5_CSR_MSTATUS_UIE ? 1 : 0;
    vm->mstatus.sie = value & R5_CSR_MSTATUS_SIE ? 1 : 0;
    vm->mstatus.mie = value & R5_CSR_MSTATUS_MIE ? 1 : 0;
    vm->mstatus.upie = value & R5_CSR_MSTATUS_UPIE ? 1 : 0;
    vm->mstatus.spie = value & R5_CSR_MSTATUS_SPIE ? 1 : 0;
    vm->mstatus.mpie = value & R5_CSR_MSTATUS_MPIE ? 1 : 0;
    vm->mstatus.spp = value & R5_CSR_MSTATUS_SPP ? 1 : 0;

    if ((value & R5_CSR_MSTATUS_MPP_U) == R5_CSR_MSTATUS_MPP_U)
        vm->mstatus.mpp = R5_MSTATUS_MPP_U;
    else if ((value & R5_CSR_MSTATUS_MPP_S) == R5_CSR_MSTATUS_MPP_S)
        vm->mstatus.mpp = R5_MSTATUS_MPP_S;
    else if ((value & R5_CSR_MSTATUS_MPP_M) == R5_CSR_MSTATUS_MPP_M)
        vm->mstatus.mpp = R5_MSTATUS_MPP_M;
    
    if ((value & R5_CSR_MSTATUS_FS_OFF) == R5_CSR_MSTATUS_FS_OFF)
        vm->mstatus.fs = R5_MSTATUS_FS_OFF;
    else if ((value & R5_CSR_MSTATUS_FS_INITIAL) == R5_CSR_MSTATUS_FS_INITIAL)
        vm->mstatus.fs = R5_MSTATUS_FS_INITIAL;
    else if ((value & R5_CSR_MSTATUS_FS_CLEAN) == R5_CSR_MSTATUS_FS_CLEAN)
        vm->mstatus.fs = R5_MSTATUS_FS_CLEAN;
    else if ((value & R5_CSR_MSTATUS_FS_DIRTY) == R5_CSR_MSTATUS_FS_DIRTY)
        vm->mstatus.fs = R5_MSTATUS_FS_DIRTY;

    if ((value & R5_CSR_MSTATUS_XS_OFF) == R5_CSR_MSTATUS_XS_OFF)
        vm->mstatus.xs = R5_MSTATUS_XS_OFF;
    else if ((value & R5_CSR_MSTATUS_XS_INITIAL) == R5_CSR_MSTATUS_XS_INITIAL)
        vm->mstatus.xs = R5_MSTATUS_XS_INITIAL;
    else if ((value & R5_CSR_MSTATUS_XS_CLEAN) == R5_CSR_MSTATUS_XS_CLEAN)
        vm->mstatus.xs = R5_MSTATUS_XS_CLEAN;
    else if ((value & R5_CSR_MSTATUS_XS_DIRTY) == R5_CSR_MSTATUS_XS_DIRTY)
        vm->mstatus.xs = R5_MSTATUS_XS_DIRTY;

    vm->mstatus.mprv = value & R5_CSR_MSTATUS_MPRV ? 1 : 0;
    vm->mstatus.sum = value & R5_CSR_MSTATUS_SUM ? 1 : 0;
    vm->mstatus.mxr = value & R5_CSR_MSTATUS_MXR ? 1 : 0;
    vm->mstatus.tvm = value & R5_CSR_MSTATUS_TVM ? 1 : 0;
    vm->mstatus.tw = value & R5_CSR_MSTATUS_TW ? 1 : 0;
    vm->mstatus.tsr = value & R5_CSR_MSTATUS_TSR ? 1 : 0;
    vm->mstatus.sd = value & R5_CSR_MSTATUS_SD ? 1 : 0;

    return 0;
}


uint32_t vm_csr_mtvec_read(struct vm * vm) {
    uint32_t mtvec = vm->mtvec.base;
    switch (vm->mtvec.mode) {
    case R5_MTVEC_DIRECT: mtvec |= R5_CSR_MTVEC_MODE_DIRECT; break;
    case R5_MTVEC_VECTORED: mtvec |= R5_CSR_MTVEC_MODE_VECTORED; break;
    }
    return mtvec;
}

int vm_csr_mtvec_write(struct vm * vm, uint32_t value) {
    vm->mtvec.base = value & R5_CSR_MTVEC_BASE_MASK;
    switch (value & R5_CSR_MTVEC_MODE_MASK) {
    case R5_CSR_MTVEC_MODE_DIRECT: vm->mtvec.mode = R5_MTVEC_DIRECT; break;
    case R5_CSR_MTVEC_MODE_VECTORED: vm->mtvec.mode = R5_MTVEC_VECTORED; break;
    }
    return 0;
}


uint32_t vm_csr_mip_read(struct vm * vm) {
    uint32_t mip = 0;
    mip |= vm->mip.usip ? R5_CSR_MIP_USIP : 0;
    mip |= vm->mip.ssip ? R5_CSR_MIP_SSIP : 0;
    mip |= vm->mip.msip ? R5_CSR_MIP_MSIP : 0;
    mip |= vm->mip.utip ? R5_CSR_MIP_UTIP : 0;
    mip |= vm->mip.stip ? R5_CSR_MIP_STIP : 0;
    mip |= vm->mip.mtip ? R5_CSR_MIP_MTIP : 0;
    mip |= vm->mip.ueip ? R5_CSR_MIP_UEIP : 0;
    mip |= vm->mip.seip ? R5_CSR_MIP_SEIP : 0;
    mip |= vm->mip.meip ? R5_CSR_MIP_MEIP : 0;
    return mip;
}

int vm_csr_mip_write(struct vm * vm, uint32_t value) {
    vm->mip.usip = value & R5_CSR_MIP_USIP ? 1 : 0;
    vm->mip.ssip = value & R5_CSR_MIP_SSIP ? 1 : 0;
    vm->mip.msip = value & R5_CSR_MIP_MSIP ? 1 : 0;
    vm->mip.utip = value & R5_CSR_MIP_UTIP ? 1 : 0;
    vm->mip.stip = value & R5_CSR_MIP_STIP ? 1 : 0;
    vm->mip.mtip = value & R5_CSR_MIP_MTIP ? 1 : 0;
    vm->mip.ueip = value & R5_CSR_MIP_UEIP ? 1 : 0;
    vm->mip.seip = value & R5_CSR_MIP_SEIP ? 1 : 0;
    vm->mip.meip = value & R5_CSR_MIP_MEIP ? 1 : 0;
    return 0;
}


uint32_t vm_csr_mie_read(struct vm * vm) {
    uint32_t mie = 0;
    mie |= vm->mie.usie ? R5_CSR_MIE_USIE : 0;
    mie |= vm->mie.ssie ? R5_CSR_MIE_SSIE : 0;
    mie |= vm->mie.msie ? R5_CSR_MIE_MSIE : 0;
    mie |= vm->mie.utie ? R5_CSR_MIE_UTIE : 0;
    mie |= vm->mie.stie ? R5_CSR_MIE_STIE : 0;
    mie |= vm->mie.mtie ? R5_CSR_MIE_MTIE : 0;
    mie |= vm->mie.ueie ? R5_CSR_MIE_UEIE : 0;
    mie |= vm->mie.seie ? R5_CSR_MIE_SEIE : 0;
    mie |= vm->mie.meie ? R5_CSR_MIE_MEIE : 0;
    return mie;
}

int vm_csr_mie_write(struct vm * vm, uint32_t value) {
    vm->mie.usie = value & R5_CSR_MIE_USIE ? 1 : 0;
    vm->mie.ssie = value & R5_CSR_MIE_SSIE ? 1 : 0;
    vm->mie.msie = value & R5_CSR_MIE_MSIE ? 1 : 0;
    vm->mie.utie = value & R5_CSR_MIE_UTIE ? 1 : 0;
    vm->mie.stie = value & R5_CSR_MIE_STIE ? 1 : 0;
    vm->mie.mtie = value & R5_CSR_MIE_MTIE ? 1 : 0;
    vm->mie.ueie = value & R5_CSR_MIE_UEIE ? 1 : 0;
    vm->mie.seie = value & R5_CSR_MIE_SEIE ? 1 : 0;
    vm->mie.meie = value & R5_CSR_MIE_MEIE ? 1 : 0;
    return 0;
}


uint32_t vm_csr_mcause_read(struct vm * vm) {
    return vm->mcause;
}

int vm_csr_mcause_write(struct vm * vm, uint32_t value) {
    vm->mcause = value;
    return OK;
}

uint32_t vm_csr_mepc_read(struct vm * vm) {
    return vm->mepc;
}

int vm_csr_mepc_write(struct vm * vm, uint32_t value) {
    vm->mepc = value;
    return OK;
}

uint32_t vm_csr_mtval_read(struct vm * vm) {
    return vm->mtval;
}

int vm_csr_mtval_write(struct vm * vm, uint32_t value) {
    vm->mtval = value;
    return OK;
}


typedef uint32_t (* vm_csr_read)(struct vm *);

vm_csr_read vm_csr_read_function(uint32_t csr) {
    switch (csr) {
    case R5_CSR_INDEX_MISA: return vm_csr_misa_read;
    case R5_CSR_INDEX_MVENDORID: return vm_csr_mvendorid_read;
    case R5_CSR_INDEX_MARCHID: return vm_csr_marchid_read;
    case R5_CSR_INDEX_MIMPID: return vm_csr_mimpid_read;
    case R5_CSR_INDEX_MHARTID: return vm_csr_mhartid_read;
    case R5_CSR_INDEX_MSTATUS: return vm_csr_mstatus_read;
    case R5_CSR_INDEX_MTVEC: return vm_csr_mtvec_read;
    case R5_CSR_INDEX_MIP: return vm_csr_mip_read;
    case R5_CSR_INDEX_MIE: return vm_csr_mie_read;
    case R5_CSR_INDEX_MCAUSE: return vm_csr_mcause_read;
    case R5_CSR_INDEX_MEPC: return vm_csr_mepc_read;
    case R5_CSR_INDEX_MTVAL: return vm_csr_mtval_read;
    }
    return NULL;
}

typedef int (* vm_csr_write)(struct vm *, uint32_t value);

vm_csr_write vm_csr_write_function(uint32_t csr) {
    switch (csr) {
    case R5_CSR_INDEX_MISA: return vm_csr_misa_write;
    case R5_CSR_INDEX_MVENDORID: return vm_csr_mvendorid_write;
    case R5_CSR_INDEX_MARCHID: return vm_csr_marchid_write;
    case R5_CSR_INDEX_MIMPID: return vm_csr_mimpid_write;
    case R5_CSR_INDEX_MHARTID: return vm_csr_mhartid_write;
    case R5_CSR_INDEX_MSTATUS: return vm_csr_mstatus_write;
    case R5_CSR_INDEX_MTVEC: return vm_csr_mtvec_write;
    case R5_CSR_INDEX_MIP: return vm_csr_mip_write;
    case R5_CSR_INDEX_MIE: return vm_csr_mie_write;
    case R5_CSR_INDEX_MCAUSE: return vm_csr_mcause_write;
    case R5_CSR_INDEX_MEPC: return vm_csr_mepc_write;
    case R5_CSR_INDEX_MTVAL: return vm_csr_mtval_write;
    }
    return NULL;
}

/* Atmoic Read and Clear Bits in CSR */
int vm_csrrc(struct vm * vm, const struct r5_instruction * ins) {
    vm_csr_read read = vm_csr_read_function(ins->csr);
    vm_csr_write write = vm_csr_write_function(ins->csr);

    if ((read == NULL) || (write == NULL)) {
        return vm_exception(vm, R5_CSR_MCAUSE_ILLEGAL_INSTRUCTION, vm->pc);
    }

    enum result result;
    if ((result = vm_set_register(vm, ins->rd, read(vm))) != OK) {
        return result;
    }
    
    write(vm, 0);

    vm->pc += 4;

    return OK;
}

/* Atmoic Read/Write Bits in CSR */
int vm_csrrw(struct vm * vm, const struct r5_instruction * ins) {
    vm_csr_read read = vm_csr_read_function(ins->csr);
    vm_csr_write write = vm_csr_write_function(ins->csr);

    if ((read == NULL) || (write == NULL)) {
        return vm_exception(vm, R5_CSR_MCAUSE_ILLEGAL_INSTRUCTION, vm->pc);
    }

    enum result result;
    uint32_t value;
    if (    ((result = vm_set_register(vm, ins->rd, read(vm))) != OK)
         || ((result = vm_get_register(vm, ins->rs1, &value)) != OK)) {
        return result;
    }

    if (ins->rs1 != R5_ZERO) {
        write(vm, value);
    }

    vm->pc += 4;

    return OK;
}

/* Atmoic Read and Set Bits in CSR */
int vm_csrrs(struct vm * vm, const struct r5_instruction * ins) {
    vm_csr_read read = vm_csr_read_function(ins->csr);
    vm_csr_write write = vm_csr_write_function(ins->csr);

    if ((read == NULL) || (write == NULL)) {
        return vm_exception(vm, R5_CSR_MCAUSE_ILLEGAL_INSTRUCTION, vm->pc);
    }

    enum result result;
    uint32_t write_value = read(vm);
    uint32_t value;
    
    if (    ((result = vm_set_register(vm, ins->rd, write_value)) != OK)
         || ((result = vm_get_register(vm, ins->rs1, &value)) != OK)) {
        return result;
    }

    if (ins->rs1 != R5_ZERO) {
        write(vm, write_value | value);
    }

    vm->pc += 4;

    return OK;
}

/* Atmoic Read and Clear Bits in CSR */
int vm_csrrci(struct vm * vm, const struct r5_instruction * ins) {
    vm_csr_read read = vm_csr_read_function(ins->csr);
    vm_csr_write write = vm_csr_write_function(ins->csr);

    if ((read == NULL) || (write == NULL)) {
        return vm_exception(vm, R5_CSR_MCAUSE_ILLEGAL_INSTRUCTION, vm->pc);
    }

    enum result result;
    if ((result = vm_set_register(vm, ins->rd, read(vm))) != OK) {
        return result;
    }

    if (ins->immediate != 0) {
        write(vm, 0);
    }

    vm->pc += 4;

    return OK;
}

/* Atmoic Read/Write Bits in CSR */
int vm_csrrwi(struct vm * vm, const struct r5_instruction * ins) {
    vm_csr_read read = vm_csr_read_function(ins->csr);
    vm_csr_write write = vm_csr_write_function(ins->csr);

    if ((read == NULL) || (write == NULL)) {
        return vm_exception(vm, R5_CSR_MCAUSE_ILLEGAL_INSTRUCTION, vm->pc);
    }

    enum result result;
    if ((result = vm_set_register(vm, ins->rd, read(vm))) != OK) {
        return result;
    }

    write(vm, ins->immediate);

    vm->pc += 4;

    return OK;
}

/* Atmoic Read and Set Bits in CSR */
int vm_csrrsi(struct vm * vm, const struct r5_instruction * ins) {
    vm_csr_read read = vm_csr_read_function(ins->csr);
    vm_csr_write write = vm_csr_write_function(ins->csr);

    if ((read == NULL) || (write == NULL)) {
        return ERROR_UNSUPPORTED_CSR;
    }

    uint32_t write_value = read(vm);

    enum result result;
    if ((result = vm_set_register(vm, ins->rd, write_value)) != OK) {
        return result;
    }

    if (ins->immediate != 0) {
        write(vm, write_value | ins->immediate);
    }

    vm->pc += 4;

    return OK;
}