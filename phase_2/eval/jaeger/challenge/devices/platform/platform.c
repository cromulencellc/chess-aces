#include "platform.h"

uint64_t * MTIME_PTR = (uint64_t *) MTIME_ADDRESS;
uint64_t * MTIMECMP_PTR = (uint64_t *) MTIMECMP_ADDRESS;

uint32_t interrupt_handler_address;
extern void (* interrupt_handler_asm)();

void set_interrupt_handler(void (* interrupt_handler)()) {
    interrupt_handler_address = (uint32_t) interrupt_handler;
    csr_mtvec_set((uint32_t) &interrupt_handler_asm);
}

void csr_mstatus_set(uint32_t mstatus) {
    asm("csrrw x0, mstatus, %0" :: "r" (mstatus));
}
void csr_mtvec_set(uint32_t mtvec) {
    asm("csrrw x0, mtvec, %0" :: "r" (mtvec));
}
void csr_mip_set(uint32_t mip) {
    asm("csrrw x0, mip, %0" :: "r" (mip));
}
void csr_mie_set(uint32_t mie) {
    asm("csrrw x0, mie, %0" :: "r" (mie));
}
void csr_mcause_set(uint32_t mcause) {
    asm("csrrw x0, mcause, %0" :: "r" (mcause));
}
void csr_mepc_set(uint32_t mepc) {
    asm("csrrw x0, mepc, %0" :: "r" (mepc));
}
void csr_mtval_set(uint32_t mtval) {
    asm("csrrw x0, mtval, %0" :: "r" (mtval));
}


uint32_t csr_mstatus_get() {
    uint32_t mstatus;
    asm("csrrs %0, mstatus, x0" : "=r" (mstatus));
    return mstatus;
}

uint32_t csr_mtvec_get() {
    uint32_t mtvec;
    asm("csrrs %0, mtvec, x0" : "=r" (mtvec));
    return mtvec;
}

uint32_t csr_mip_get() {
    uint32_t mip;
    asm("csrrs %0, mip, x0" : "=r" (mip));
    return mip;
}

uint32_t csr_mie_get() {
    uint32_t mie;
    asm("csrrs %0, mie, x0" : "=r" (mie));
    return mie;
}

uint32_t csr_mcause_get() {
    uint32_t mcause;
    asm("csrrs %0, mcause, x0" : "=r" (mcause));
    return mcause;
}

uint32_t csr_mepc_get() {
    uint32_t mepc;
    asm("csrrs %0, mepc, x0" : "=r" (mepc));
    return mepc;
}

uint32_t csr_mtval_get() {
    uint32_t mtval;
    asm("csrrs %0, mtval, x0" : "=r" (mtval));
    return mtval;
}


void wait_for_interrupt() {
    asm("wfi");
}