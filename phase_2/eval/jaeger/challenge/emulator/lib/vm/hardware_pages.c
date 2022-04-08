#include "hardware_pages.h"

#include <stdlib.h>

#include "error.h"

struct hardware_page * hardware_page_create() {
    struct hardware_page * hardware_page = malloc(sizeof(struct hardware_page));

    if (hardware_page == NULL) {
        return NULL;
    }

    unsigned int i;
    for (i = 0; i < R5_HARDWARE_PAGE_LEVEL_ENTRIES; i++) {
        hardware_page->pages[i] = NULL;
    }

    return hardware_page;
}


void hardware_page_delete(struct hardware_page * hardware_page) {
    unsigned int i;

    for (i = 0; i < R5_HARDWARE_PAGE_LEVEL_ENTRIES; i++) {
        if (hardware_page->pages[i] != NULL) {
            free(hardware_page->pages[i]);
        }
    }

    free(hardware_page);
}


struct hardware_pages * hardware_pages_create() {
    struct hardware_pages * hardware_pages =
        malloc(sizeof(struct hardware_pages));
    
    if (hardware_pages == NULL) {
        return NULL;
    }

    unsigned int i;
    for (i = 0; i < R5_HARDWARE_PAGE_LEVEL_ENTRIES; i++) {
        hardware_pages->pages[i] = NULL;
    }

    return hardware_pages;
}


void hardware_pages_delete(struct hardware_pages * hardware_pages) {
    unsigned int i;

    for (i = 0; i < R5_HARDWARE_PAGE_LEVEL_ENTRIES; i++) {
        if (hardware_pages->pages[i] != NULL) {
            hardware_page_delete(hardware_pages->pages[i]);
        }
    }

    free(hardware_pages);
}


enum result hardware_pages_create_page(
    struct hardware_pages * hardware_pages,
    uint32_t address
) {
    if (address & (R5_HARDWARE_PAGE_SIZE - 1)) {
        return ERROR_NONALIGNED_PAGE;
    }

    uint32_t level_1 = address >> R5_HARDWARE_PAGE_LEVEL_BITS;
    level_1 >>= R5_HARDWARE_PAGE_BITS;
    uint32_t level_2 = address >> R5_HARDWARE_PAGE_BITS;

    level_1 &= R5_HARDWARE_PAGE_LEVEL_MASK;
    level_2 &= R5_HARDWARE_PAGE_LEVEL_MASK;

    struct hardware_page * hardware_page;
    if (hardware_pages->pages[level_1] == NULL) {
        hardware_page = hardware_page_create();
        if (hardware_page == NULL) {
            return ERROR_OOM;
        }
        hardware_pages->pages[level_1] = hardware_page;
    }
    else {
        hardware_page = hardware_pages->pages[level_1];
    }

    if (hardware_page->pages[level_2] != NULL) {
        return ERROR_PAGE_EXISTS;
    }

    hardware_page->pages[level_2] = malloc(R5_HARDWARE_PAGE_SIZE);
    if (hardware_page->pages[level_2] == NULL) {
        return ERROR_OOM;
    }

    return OK;
}


uint8_t * hardware_pages_get_page(
    struct hardware_pages * hardware_pages,
    uint32_t address
) {
    uint32_t level_1 = address >> R5_HARDWARE_PAGE_LEVEL_BITS;
    level_1 >>= R5_HARDWARE_PAGE_BITS;
    uint32_t level_2 = address >> R5_HARDWARE_PAGE_BITS;

    level_1 &= R5_HARDWARE_PAGE_LEVEL_MASK;
    level_2 &= R5_HARDWARE_PAGE_LEVEL_MASK;

    struct hardware_page * hardware_page = hardware_pages->pages[level_1];
    if (hardware_page == NULL) {
        return NULL;
    }
    
    return hardware_page->pages[level_2];
}

