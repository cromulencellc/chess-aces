#ifndef r5_csr_HEADER
#define r5_csr_HEADER

#include <stdint.h>

extern uint32_t exception_code_priority[];

/**
 * Given two MCAUSE exception codes, returns the higher priority
 * exception code.
 * @param a The first valid MCAUSE exception code.
 * @param b The second valid MCAUSE exception code.
 * @return The exception code with the higher priority.
 */
uint32_t get_higher_priority_exception_code(uint32_t a, uint32_t b);

/* These are the values held in the CSR portion of instructions which interact
   with CSR registers. */
#define R5_CSR_INDEX_USTATUS 0x000
#define R5_CSR_INDEX_UIE 0x004
#define R5_CSR_INDEX_UTVEC 0x005
#define R5_CSR_INDEX_USCRATCH 0x040
#define R5_CSR_INDEX_UEPC 0x041
#define R5_CSR_INDEX_UCAUSE 0x042
#define R5_CSR_INDEX_UTVAL 0x043
#define R5_CSR_INDEX_UIP 0x044

#define R5_CSR_INDEX_FFLAGS 0x001
#define Rc_CSR_INDEX_FRM 0x002
#define R5_CSR_INDEX_FCSR 0x003

#define R5_CSR_INDEX_CYCLE 0xc00
#define R5_CSR_INDEX_TIME 0xc01
#define R5_CSR_INDEX_INSTRET 0xc02
#define R5_CSR_INDEX_HPMCOUNTER3 0xc03
#define R5_CSR_INDEX_HPMCOUNTER4 0xc04
#define R5_CSR_INDEX_CYCLEH 0xc80
#define R5_CSR_INDEX_TIMEH 0xc81
#define R5_CSR_INDEX_INSTRETH 0xc82
#define R5_CSR_INDEX_HPMCOUNTER3H 0xc83
#define R5_CSR_INDEX_HPMCOUNTER4H 0xc84

#define R5_CSR_INDEX_SSTATUS 0x100
#define R5_CSR_INDEX_SEDELEG 0x102
#define R5_CSR_INDEX_SIDELEG 0x103
#define R5_CSR_INDEX_SIE 0x104
#define R5_CSR_INDEX_STVEC 0x105
#define R5_CSR_INDEX_SCOUNTEREN 0x106

#define R5_CSR_INDEX_SSCRATCH 0x140
#define R5_CSR_INDEX_SEPC 0x141
#define R5_CSR_INDEX_SCAUSE 0x142
#define R5_CSR_INDEX_STVAL 0x143
#define R5_CSR_INDEX_SIP 0x144

#define R5_CSR_INDEX_SATP 0x180

#define R5_CSR_INDEX_MVENDORID 0Xf11
#define R5_CSR_INDEX_MARCHID 0xf12
#define R5_CSR_INDEX_MIMPID 0xf13
#define R5_CSR_INDEX_MHARTID 0xf14
#define R5_CSR_INDEX_MSTATUS 0x300
#define R5_CSR_INDEX_MISA 0x301
#define R5_CSR_INDEX_MEDELEG 0x302
#define R5_CSR_INDEX_MIDELEG 0x303
#define R5_CSR_INDEX_MIE 0x304
#define R5_CSR_INDEX_MTVEC 0x305
#define R5_CSR_INDEX_MCOUNTEREN 0x306

#define R5_CSR_INDEX_MSCRATCH 0x340
#define R5_CSR_INDEX_MEPC 0x341
#define R5_CSR_INDEX_MCAUSE 0x342
#define R5_CSR_INDEX_MTVAL 0x343
#define R5_CSR_INDEX_MIP 0x344
#define R5_CSR_INDEX_PMPCFG0 0x3a0
#define R5_CSR_INDEX_PMPCFG1 0x3a1
#define R5_CSR_INDEX_PMPCFG2 0x3a2
#define R5_CSR_INDEX_PMPCFG3 0x3a3
#define R5_CSR_INDEX_PMPADDR0 0x3b0
#define R5_CSR_INDEX_PMPADDR1 0x3b1
#define R5_CSR_INDEX_PMPADDR2 0x3b2
#define R5_CSR_INDEX_PMPADDR3 0x3b3


#define R5_CSR_INDEX_MCYCLE 0xb00
#define R5_CSR_INDEX_MINSTRET 0xb02
#define R5_CSR_INDEX_MHPMCOUNTER3 0xb03
#define R5_CSR_INDEX_MHPMCOUNTER4 0xb04
#define R5_CSR_INDEX_MCYCLEH 0xb80
#define R5_CSR_INDEX_MINSTRETH 0xb82
#define R5_CSR_INDEX_MHPMCOUNTER3H 0xb83
#define R5_CSR_INDEX_MHPMCOUNTER4H 0xb84
#define R5_CSR_INDEX_MCOUNTINHIBIT 0x320
#define R5_CSR_INDEX_MHPMEVENT3 0x323
#define R5_CSR_INDEX_MHPMEVENT4 0x324
#define R5_CSR_INDEX_TSELECT 0x7a0
#define R5_CSR_INDEX_TDATA1 0x7a1
#define R5_CSR_INDEX_TDATA2 0x7a2
#define R5_CSR_INDEX_TDATA3 0x7a3
#define R5_CSR_INDEX_DCSR 0x7b0
#define R5_CSR_INDEX_DPC 0x7b1
#define R5_CSR_INDEX_DSCRATCH0 0x7b2
#define R5_CSR_INDEX_DSCRATCH1 0x7b3

/*
* Values for MISA register
*/


#define R5_CSR_MISA_MXL_32 (0x1 << 30)
#define R5_CSR_MISA_MXL_64 (0x2 << 30)
#define R5_CSR_MISA_MXL_128 (0x3 << 30)
#define R5_CSR_MISA_MXLEN_32 (0x1 << 26)
#define R5_CSR_MISA_MXLEN_64 (0x2 << 26)
#define R5_CSR_MISA_MXLEN_128 (0x3 << 26)

#define R5_CSR_MISA_A (1 << 0)
#define R5_CSR_MISA_B (1 << 1)
#define R5_CSR_MISA_C (1 << 2)
#define R5_CSR_MISA_D (1 << 3)
#define R5_CSR_MISA_E (1 << 4)
#define R5_CSR_MISA_F (1 << 5)
#define R5_CSR_MISA_G (1 << 6)
#define R5_CSR_MISA_H (1 << 7)
#define R5_CSR_MISA_I (1 << 8)
#define R5_CSR_MISA_J (1 << 9)
#define R5_CSR_MISA_K (1 << 10)
#define R5_CSR_MISA_L (1 << 11)
#define R5_CSR_MISA_M (1 << 12)
#define R5_CSR_MISA_N (1 << 13)
#define R5_CSR_MISA_O (1 << 14)
#define R5_CSR_MISA_P (1 << 15)
#define R5_CSR_MISA_Q (1 << 16)
#define R5_CSR_MISA_R (1 << 17)
#define R5_CSR_MISA_S (1 << 18) /* Supervisor mode implemented */
#define R5_CSR_MISA_T (1 << 19)
#define R5_CSR_MISA_U (1 << 20) /* User mode implemented */
#define R5_CSR_MISA_V (1 << 21)
#define R5_CSR_MISA_W (1 << 22)
#define R5_CSR_MISA_X (1 << 23)
#define R5_CSR_MISA_Y (1 << 24)
#define R5_CSR_MISA_Z (1 << 25)


#define R5_CSR_MISA_VALUE (R5_CSR_MISA_MXL_32 | \
                           R5_CSR_MISA_MXLEN_32 | \
                           R5_CSR_MISA_I | \
                           R5_CSR_MISA_M)


/* Values for various machine registers we can safely set to 0 */

#define R5_CSR_MVENDORID_VALUE 0
#define R5_CSR_MARCHID_VALUE 0
#define R5_CSR_MIMPID_VALUE 0
/* Hardware Thread ID. We have one core/thread, so this can always be 0. */
#define R5_CSR_MHARTID_VALUE 0

/* Values for MSTATUS */

#define R5_CSR_MSTATUS_UIE  (1 << 0) /* Enable user interrupts */
#define R5_CSR_MSTATUS_SIE  (1 << 1) /* Enable system interrupts */
#define R5_CSR_MSTATUS_MIE  (1 << 3) /* Enable machine interrupts */
#define R5_CSR_MSTATUS_UPIE (1 << 4) /* Value of UIE prior to the trap */
#define R5_CSR_MSTATUS_SPIE (1 << 5) /* Value of SIE prior to the trap */
#define R5_CSR_MSTATUS_MPIE (1 << 7) /* Value of MIE prior to the trap */
#define R5_CSR_MSTATUS_SPP  (1 << 8) 
#define R5_CSR_MSTATUS_MPP_U (0 << 11)
#define R5_CSR_MSTATUS_MPP_S (1 << 11)
#define R5_CSR_MSTATUS_MPP_M (3 << 11)
/* Information about the state of fp registers */
#define R5_CSR_MSTATUS_FS_OFF (0 << 13)
#define R5_CSR_MSTATUS_FS_INITIAL (1 << 13)
#define R5_CSR_MSTATUS_FS_CLEAN (2 << 13)
#define R5_CSR_MSTATUS_FS_DIRTY (3 << 13)
/* Information about the state of special extension registers */
#define R5_CSR_MSTATUS_XS_OFF (0 << 15)
#define R5_CSR_MSTATUS_XS_INITIAL (1 << 15)
#define R5_CSR_MSTATUS_XS_CLEAN (2 << 15)
#define R5_CSR_MSTATUS_XS_DIRTY (3 << 15)
/* When MPRV == 1, memory accesses take place at the privilege level set in MPP
*/
#define R5_CSR_MSTATUS_MPRV (1 << 17)
/* Supervisor User Memory. When 0, supervisor access to pages where U=1 will
   fault. */
#define R5_CSR_MSTATUS_SUM (1 << 18)
/* Make executable pages readable. When 0, page reads will fail if page is not
   readable. When 1, page reads will succeed if executable, regardless of read
   bit. */
#define R5_CSR_MSTATUS_MXR (1 << 19)
/* Trap Virtual Memory, virtualization support */
#define R5_CSR_MSTATUS_TVM (1 << 20)
/* Timeout Wait. When TW=0, WFI instruction may executie in lower privilege
   modes. When TW=1, WFI will cause illegal instruction within an
   implementation-specific, bounded time. */
#define R5_CSR_MSTATUS_TW (1 << 21)
/* Trap SRET. When TSR=1, SRET will raise illegal instruction while in
   supervisor mode. */
#define R5_CSR_MSTATUS_TSR (1 << 22)
/* Summarizes whether either FS or XS fields signal presence of some dirty state
   that require saving extended user context to memory. */
#define R5_CSR_MSTATUS_SD (1 << 31)


/* MTVEC, this is the Trap Vector Table */
#define R5_CSR_MTVEC_BASE_MASK 0xfffffffc
#define R5_CSR_MTVEC_MODE_MASK 0x00000003

#define R5_CSR_MTVEC_MODE_DIRECT 0
#define R5_CSR_MTVEC_MODE_VECTORED 1


/* Interrupt Pending register */
#define R5_CSR_MIP_USIP (1 << 0)
#define R5_CSR_MIP_SSIP (1 << 1)
#define R5_CSR_MIP_MSIP (1 << 3)
#define R5_CSR_MIP_UTIP (1 << 4)
#define R5_CSR_MIP_STIP (1 << 5)
#define R5_CSR_MIP_MTIP (1 << 6)
#define R5_CSR_MIP_UEIP (1 << 7)
#define R5_CSR_MIP_SEIP (1 << 8)
#define R5_CSR_MIP_MEIP (1 << 11)


/* Interrupt Enabled Register */
#define R5_CSR_MIE_USIE (1 << 0)
#define R5_CSR_MIE_SSIE (1 << 1)
#define R5_CSR_MIE_MSIE (1 << 3)
#define R5_CSR_MIE_UTIE (1 << 4)
#define R5_CSR_MIE_STIE (1 << 5)
#define R5_CSR_MIE_MTIE (1 << 7)
#define R5_CSR_MIE_UEIE (1 << 8)
#define R5_CSR_MIE_SEIE (1 << 9)
#define R5_CSR_MIE_MEIE (1 << 11)


/* Machine Cause Register */
#define R5_CSR_MCAUSE_INTERRUPT 0x80000000

#define R5_CSR_MCAUSE_USER_SOFTWARE_INTERRUPT (R5_CSR_MCAUSE_INTERRUPT | 0)
#define R5_CSR_MCAUSE_SUPERVISOR_SOFTWARE_INTERRUPT (R5_CSR_MCAUSE_INTERRUPT | 1)
#define R5_CSR_MCAUSE_MACHINE_SOFTWARE_INTERRUPT (R5_CSR_MCAUSE_INTERRUPT | 3)

#define R5_CSR_MCAUSE_USER_TIMER_INTERRUPT (R5_CSR_MCAUSE_INTERRUPT | 4)
#define R5_CSR_MCAUSE_SUPERVISOR_TIMER_INTERRUPT (R5_CSR_MCAUSE_INTERRUPT | 5)
#define R5_CSR_MCAUSE_MACHINE_TIMER_INTERRUPT (R5_CSR_MCAUSE_INTERRUPT | 7)
#define R5_CSR_MCAUSE_USER_EXTERNAL_INTERRUPT (R5_CSR_MCAUSE_INTERRUPT | 8)
#define R5_CSR_MCAUSE_SUPERVISOR_EXTERNAL_INTERRUPT (R5_CSR_MCAUSE_INTERRUPT | 9)
#define R5_CSR_MCAUSE_MACHINE_EXTERNAL_INTERRUPT (R5_CSR_MCAUSE_INTERRUPT | 11)
#define R5_CSR_MCAUSE_INSTRUCTION_ADDRESS_MISALIGNED 0
#define R5_CSR_MCAUSE_INSTRUCTION_ACCESS_FAULT 1
#define R5_CSR_MCAUSE_ILLEGAL_INSTRUCTION 2
#define R5_CSR_MCAUSE_BREAKPOINT 3
#define R5_CSR_MCAUSE_LOAD_ADDRESS_MISALIGNED 4
#define R5_CSR_MCAUSE_LOAD_ACCESS_FAULT 5
#define R5_CSR_MCAUSE_STORE_AMO_ADDRESS_MISALIGNED 6
#define R5_CSR_MCAUSE_STORE_AMO_ACCESS_FAULT 7
#define R5_CSR_MCAUSE_ENVIRONMENT_CALL_FROM_U_MODE 8
#define R5_CSR_MCAUSE_ENVIRONMENT_CALL_FROM_S_MODE 9
#define R5_CSR_MCAUSE_ENVIRONMENT_CALL_FROM_M_MODE 11
#define R5_CSR_MCAUSE_INSTRUCTION_PAGE_FAULT 12
#define R5_CSR_MCAUSE_LOAD_PAGE_FAULT 13
#define R5_CSR_MCAUSE_STORE_AMO_PAGE_FAULT 15

#endif