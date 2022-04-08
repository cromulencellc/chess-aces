#ifndef vm_HEADER
#define vm_HEADER

#include <stdint.h>

#include "csr.h"
#include "devices/device.h"
#include "hardware_pages.h"
#include "r5/instruction.h"


enum vm_step_result {
    VM_OK,
    SEGMENTATION_FAULT,
    ILLEGAL_INSTRUCTION,
    UNSUPPORTED_INSTRUCTION,
    DIVISION_BY_ZERO,
    ECALL
};


#define VM_MAX_DEVICES 8
#define DEVICES_BASE_ADDRESS 0x40000000
#define DEVICE_ADDRESS_OFFSET 0x00010000
#define DEVICE_0_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICE_ADDRESS_OFFSET * 0))
#define DEVICE_1_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICE_ADDRESS_OFFSET * 1))
#define DEVICE_2_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICE_ADDRESS_OFFSET * 2))
#define DEVICE_3_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICE_ADDRESS_OFFSET * 3))
#define DEVICE_4_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICE_ADDRESS_OFFSET * 4))
#define DEVICE_5_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICE_ADDRESS_OFFSET * 5))
#define DEVICE_6_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICE_ADDRESS_OFFSET * 6))
#define DEVICE_7_ADDRESS (DEVICES_BASE_ADDRESS + (DEVICE_ADDRESS_OFFSET * 7))

#define MTIME_ADDRESS 0x02000000
#define MTIMECMP_ADDRESS 0x02000008
#define MTIME_BYTE_SIZE 8
#define MTIMECMP_BYTE_SIZE 8

/** A quick look up table for base addresses of different devices. */

struct vm_device_address {
    uint32_t lo;
    uint32_t hi;
};

extern struct vm_device_address VM_DEVICE_ADDRESSES[VM_MAX_DEVICES];

enum vm_level {
    VM_LEVEL_USER,
    VM_LEVEL_SUPERVISOR,
    VM_LEVEL_RESERVED,
    VM_LEVEL_MACHINE
};

struct vm {
    uint32_t regs[R5_NUMBER_GPRS];
    uint32_t pc;
    struct r5_csr_mstatus mstatus;
    struct r5_csr_mtvec mtvec;
    struct r5_csr_mip mip;
    struct r5_csr_mie mie;
    uint64_t mtime; /* mtime and mtimecmp are memory-mapped registers */
    uint64_t mtimecmp;
    uint32_t mepc; /* Machine Exception Program Counter */
    uint32_t mcause;
    uint32_t mtval;
    enum vm_level level;
    struct hardware_pages * hardware_pages;
    uint32_t num_devices;
    struct device * devices[VM_MAX_DEVICES];

    /*
    * When mtime >= mtimecmp, the timer interrupt triggers (if enabled). That
    * interrupted remains posted until mtimecmp is re-written. If this is set to
    * 1, mtimecmp has been written, and the interrupt can be fired. Once the
    * timer interrupt is written, this is reset to 0 until mtimecmp is
    * rewritten.
    */
    int mtimecmp_written;

    /** If set to 1, we print out trace information */
    int tracing;

    /** If set to 1, we are waiting for an interrupt */
    int wfi;
};


/**
 * Create a new vm
 * @return NULL on ERROR_OOM, a new vm on success
 */
struct vm * vm_create();

/**
 * Create a new vm, and load the given elf file to set it up
 * @param path The path to the elf we will use to create the vm.
 * @return NULL on error, or an instantiated VM.
 */
struct vm * vm_from_elf(const char * path);

/**
 * Delete a vm, freeing all memory associated with the VM
 * @param vm A valid, initialized vm.
 */
void vm_delete(struct vm *);

/**
 * Loads an elf containing the program for the VM.
 *
 * Sets GP register to __global_pointer$
 * @param vm
 * @param elf The bytes of an RV32IM big endian binary
 * @return OK on success, non-zero on error
 */
enum result vm_load_elf(struct vm * vm, const uint8_t * elf);

/**
 * Adds a device to the vm.
 * @param vm
 * @param device The device to add
 * @return OK on success, or ERROR_TOO_MANY_DEVICES
 */
int vm_add_device(struct vm * vm, struct device * device);

/**
 * Executing pending actions on this vm's devices
 * @param vm
 */
int vm_run_devices(struct vm * vm);

/**
 * Create memory for a given page
 * @param address A page-aligned address
 * @return OK on success, or ERROR_OOM, ERROR_NONALIGNED_PAGE, ERROR_PAGE_EXISTS.
 */
int vm_create_hardware_page(struct vm *, uint32_t address);

/**
 * Causes the vm to raise an exception/interrupt
 * @param vm 
 * @param mcause The cause of the exception. mcause will be set to this value.
 * @param mtval The value of mtval.
 * @return 0 on success. Does not fail.
 */
enum result vm_exception(struct vm * vm, uint32_t mcause, uint32_t mtval);

/**
 * Calling this increases mtime by 1 in the vm. If necessary, will trigger a
 * timer interrupt.
 * @param vm
 */
enum result vm_timer_tick(struct vm * vm);

int vm_get_register(struct vm * vm, enum r5_register reg, uint32_t * value);
int vm_set_register(struct vm * vm, enum r5_register reg, uint32_t value);

int vm_get_pc(struct vm * vm, uint32_t * value);
int vm_set_pc(struct vm * vm, uint32_t value);

/**
 * Step the VM forward one instruction.
 * @param vm A properly initialized vm instance.
 * @return 0 on success, non-zero on error.
 */
int vm_step(struct vm * vm);

#endif