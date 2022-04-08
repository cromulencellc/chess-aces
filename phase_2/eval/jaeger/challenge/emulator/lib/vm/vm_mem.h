#ifndef vm_mem_HEADER
#define vm_mem_HEADER

#include <stdint.h>

struct vm;

/* Memory operations for vm */


/**
 * Returns a single byte from a hardware page.
 * @param address The address to retrieve the byte from
 * @param byte The found value will be returned in byte
 * @return OK on success, or ERROR_PAGE_DOES_NOT_EXIST
 */
enum result vm_get_u8(struct vm * vm, uint32_t address, uint8_t * byte);

/**
 * Sets a single byte in a hardware page
 * @param address The address where we wish to set the byte
 * @param byte The byte to set
 * @return OK on success, or ERROR_PAGE_DOES_NOT_EXIST
 */
enum result vm_set_u8(struct vm * vm, uint32_t address, uint8_t byte);

enum result vm_get_u16(struct vm * vm, uint32_t address, uint16_t * half);
enum result vm_get_u32(struct vm * vm, uint32_t address, uint32_t * word);
enum result vm_set_u16(struct vm * vm, uint32_t address, uint16_t half);
enum result vm_set_u32(struct vm * vm, uint32_t address, uint32_t word);

enum result vm_set_buf(
    struct vm * vm,
    uint32_t address,
    const uint8_t * buf,
    uint32_t buf_size
);
enum result vm_get_buf(
    struct vm * vm,
    uint32_t address,
    uint8_t * buf,
    uint32_t buf_size
);

#endif