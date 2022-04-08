#include "csr.h"

uint32_t exception_code_priority[] = {
    /*
        If we had support for hardware breakpoints/watchpoints, they would
        be the highest priority exception.
        R5_CSR_MCAUSE_INSTRUCTION_ADDRESS BREAKPOINT ,
    */
    R5_CSR_MCAUSE_INSTRUCTION_PAGE_FAULT,
    R5_CSR_MCAUSE_INSTRUCTION_ACCESS_FAULT,
    R5_CSR_MCAUSE_ILLEGAL_INSTRUCTION,
    R5_CSR_MCAUSE_INSTRUCTION_ADDRESS_MISALIGNED,
    R5_CSR_MCAUSE_ENVIRONMENT_CALL_FROM_M_MODE,
    R5_CSR_MCAUSE_ENVIRONMENT_CALL_FROM_S_MODE,
    R5_CSR_MCAUSE_ENVIRONMENT_CALL_FROM_U_MODE,
    R5_CSR_MCAUSE_BREAKPOINT,
    /*
        Again, we don't have hardware breakpoint, so we're missing another
        exception in our priority list here.
    */
   R5_CSR_MCAUSE_STORE_AMO_ADDRESS_MISALIGNED,
   R5_CSR_MCAUSE_LOAD_ADDRESS_MISALIGNED,
   R5_CSR_MCAUSE_STORE_AMO_PAGE_FAULT,
   R5_CSR_MCAUSE_LOAD_PAGE_FAULT,
   R5_CSR_MCAUSE_STORE_AMO_ACCESS_FAULT,
   R5_CSR_MCAUSE_LOAD_ACCESS_FAULT
};


uint32_t get_higher_priority_exception_code(uint32_t a, uint32_t b) {
    unsigned int i;
    for (i = 0; i < sizeof(exception_code_priority) / sizeof(uint32_t); i++) {
        if (exception_code_priority[i] == a) {
            return a;
        }
        else if (exception_code_priority[i] == b) {
            return b;
        }
    }
    return a;
}