#ifndef vm_hardware_page_HEADER
#define vm_hardware_page_HEADER

/*
    Hardware pages are pages we use for storing memory in the emulator (physical
    RAM). They are not Risc-V pages.

    Three levels. First two levels hold HARDWARE_PAGE_LEVEL_ENTRIES number of
    children. The final level holds HARDWARE_PAGE_SIZE bytes.
*/

#define R5_HARDWARE_PAGE_BITS 12
#define R5_HARDWARE_PAGE_LEVEL_BITS 10

#define R5_HARDWARE_PAGE_LEVEL_ENTRIES (1 << R5_HARDWARE_PAGE_LEVEL_BITS)
#define R5_HARDWARE_PAGE_LEVEL_MASK (R5_HARDWARE_PAGE_LEVEL_ENTRIES - 1)
#define R5_HARDWARE_PAGE_SIZE (1 << R5_HARDWARE_PAGE_BITS)
#define R5_HARDWARE_PAGE_MASK (R5_HARDWARE_PAGE_SIZE - 1)

#define R5_NUMBER_GPRS 32

#include <stdint.h>

#include "result.h"


struct hardware_page {
    uint8_t * pages[R5_HARDWARE_PAGE_LEVEL_ENTRIES];
};


/**
 * Create a new hardware page
 * @return NULL on ERROR_OOM, a new hardware page on success
 */
struct hardware_page * hardware_page_create();

/**
 * Delete a hardware page, freeing all memory held by this hardware page
 * @param hardware_page A valid, initialized hardware page.
 */
void hardware_page_delete(struct hardware_page *);


struct hardware_pages {
    struct hardware_page * pages[R5_HARDWARE_PAGE_LEVEL_ENTRIES];
};

struct hardware_pages * hardware_pages_create();
void hardware_pages_delete(struct hardware_pages * hardware_pages);

/**
 * Create a new leaf page at the given address.
 * @param hardware_pages An initialized hardware_pages.
 * @param address An aligned, non-existant page to create.
 * @return OK on success, ERROR_NONALIGNED_PAGE, ERROR_PAGE_EXISTS, or ERROR_OOM
 */
enum result hardware_pages_create_page(
    struct hardware_pages * hardware_pages,
    uint32_t address
);

/**
 * Fetch the page for the given address. The given address does not need to be
 * page aligned.
 * @param hardware_pages An initialized hardware_pages.
 * @param address An address which resides within an existing page.
 * @return The bytes of the page if it exists, or NULL if the page does not
 * exist.
 */
uint8_t * hardware_pages_get_page(
    struct hardware_pages * hardware_pages,
    uint32_t address
);

#endif