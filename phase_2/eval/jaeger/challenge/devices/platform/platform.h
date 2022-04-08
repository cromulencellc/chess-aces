#ifndef platform_HEADER
#define platform_HEADER

#define DEVICES_BASE_ADDRESS 0x40000000
#define DEVICES_ADDRESS_OFFSET 0x00010000

#define DEVICE_0_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICES_ADDRESS_OFFSET * 0))
#define DEVICE_1_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICES_ADDRESS_OFFSET * 1))
#define DEVICE_2_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICES_ADDRESS_OFFSET * 2))
#define DEVICE_3_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICES_ADDRESS_OFFSET * 3))
#define DEVICE_4_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICES_ADDRESS_OFFSET * 4))
#define DEVICE_5_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICES_ADDRESS_OFFSET * 5))

#define READ_DATA_READY_BIT 1
#define READ_DATA_COMPLETE_BIT 2
#define WRITE_DATA_READY_BIT 4
#define WRITE_DATA_COMPLETE_BIT 8

#define STATUS_BYTE_OFFSET 0
#define READ_SIZE_OFFSET 1
#define WRITE_SIZE_OFFSET 2
#define READ_BUFFER_OFFSET 0x100
#define WRITE_BUFFER_OFFSET 0x200
#define READ_BUFFER_SIZE 16
#define WRITE_BUFFER_SIZE 16

#define DEVICE_STATUS(DEVICE_ADDRESS) \
    ((volatile uint8_t *) (DEVICE_ADDRESS + STATUS_BYTE_OFFSET))
#define DEVICE_READ_SIZE(DEVICE_ADDRESS) \
    ((volatile uint8_t *) (DEVICE_ADDRESS + READ_SIZE_OFFSET))
#define DEVICE_WRITE_SIZE(DEVICE_ADDRESS) \
    ((volatile uint8_t *) (DEVICE_ADDRESS + WRITE_SIZE_OFFSET))
#define DEVICE_READ_BUFFER(DEVICE_ADDRESS) \
    ((volatile uint8_t *) (DEVICE_ADDRESS + READ_BUFFER_OFFSET))
#define DEVICE_WRITE_BUFFER(DEVICE_ADDRESS) \
    ((volatile uint8_t *) (DEVICE_ADDRESS + WRITE_BUFFER_OFFSET))

#define WRITE_DATA_READY(DEVICE_ADDRESS) \
    ((*DEVICE_STATUS(DEVICE_ADDRESS) & WRITE_DATA_READY_BIT) ? 1 : 0)
#define WRITE_DATA_COMPLETE(DEVICE_ADDRESS) \
    ((*DEVICE_STATUS(DEVICE_ADDRESS) & WRITE_DATA_COMPLETE_BIT) ? 1 : 0)
#define READ_DATA_READY(DEVICE_ADDRESS) \
    ((*DEVICE_STATUS(DEVICE_ADDRESS) & READ_DATA_READY_BIT) ? 1 : 0)
#define READ_DATA_COMPLETE(DEVICE_ADDRESS) \
    ((*DEVICE_STATUS(DEVICE_ADDRESS) & READ_DATA_COMPLETE_BIT) ? 1 : 0)

#define WRITE_DATA_READY_SET(DEVICE_ADDRESS, VALUE) \
    do { \
        *DEVICE_STATUS(DEVICE_ADDRESS) = \
            *DEVICE_STATUS(DEVICE_ADDRESS) & (~WRITE_DATA_READY_BIT); \
        if (VALUE) { *DEVICE_STATUS(DEVICE_ADDRESS) |= WRITE_DATA_READY_BIT; } \
    } while (0);

#define WRITE_DATA_COMPLETE_SET(DEVICE_ADDRESS, VALUE) \
    do { \
        *DEVICE_STATUS(DEVICE_ADDRESS) = \
            *DEVICE_STATUS(DEVICE_ADDRESS) & (~WRITE_DATA_COMPLETE_BIT); \
        if (VALUE) { *DEVICE_STATUS(DEVICE_ADDRESS) |= WRITE_DATA_COMPLETE_BIT; } \
    } while (0);

#define READ_DATA_READY_SET(DEVICE_ADDRESS, VALUE) \
    do { \
        *DEVICE_STATUS(DEVICE_ADDRESS) = \
            *DEVICE_STATUS(DEVICE_ADDRESS) & (~READ_DATA_READY_BIT); \
        if (VALUE) { *DEVICE_STATUS(DEVICE_ADDRESS) |= READ_DATA_READY_BIT; } \
    } while (0);

#define READ_DATA_COMPLETE_SET(DEVICE_ADDRESS, VALUE) \
    do { \
        *DEVICE_STATUS(DEVICE_ADDRESS) = \
            *DEVICE_STATUS(DEVICE_ADDRESS) & (~READ_DATA_COMPLETE_BIT); \
        if (VALUE) { *DEVICE_STATUS(DEVICE_ADDRESS) |= READ_DATA_COMPLETE_BIT; } \
    } while (0);

#define DEBUG_DEVICE_WRITE_OFFSET 0
#define DEBUG_DEVICE_BUF_OFFSET 0x100
#define DEBUG_DEVICE_BUF_SIZE 0x100

#define DEBUG_DEVICE_SEND_STRING 1
#define DEBUG_DEVICE_SEND_UINT8 2
#define DEBUG_DEVICE_SEND_UINT32 3

/* One tick in mtime is equal to this many microseconds */
#define TICK_MICROSECOND_SCALE 10000

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;


#define MTIME_ADDRESS 0x02000000
#define MTIMECMP_ADDRESS 0x02000008

extern uint64_t * MTIME_PTR;
extern uint64_t * MTIMECMP_PTR;

#define NULL ((void *) 0)

void set_interrupt_handler(void (* interrupt_handler)());

void csr_mstatus_set(uint32_t mstatus);
void csr_mtvec_set(uint32_t mtvec);
void csr_mip_set(uint32_t mip);
void csr_mie_set(uint32_t mie);
void csr_mcause_set(uint32_t mcause);
void csr_mepc_set(uint32_t mepc);
void csr_mtval_set(uint32_t mtval);

uint32_t csr_mstatus_get();
uint32_t csr_mtvec_get();
uint32_t csr_mip_get();
uint32_t csr_mie_get();
uint32_t csr_mcause_get();
uint32_t csr_mepc_get();
uint32_t csr_mtval_get();

void wait_for_interrupt();

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


#endif