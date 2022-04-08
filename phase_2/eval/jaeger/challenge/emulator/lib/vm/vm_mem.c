#include "vm_mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hardware_pages.h"
#include "log.h"
#include "vm.h"


enum result vm_get_u8(struct vm * vm, uint32_t address, uint8_t * byte) {
    uint8_t * page = hardware_pages_get_page(vm->hardware_pages, address);

    if (page == NULL) {
        /* Try devices */
        int device_found = 0;
        unsigned int i;
        for (i = 0; i < vm->num_devices; i++) {
            if (    (address < VM_DEVICE_ADDRESSES[i].lo)
                 || (address >= VM_DEVICE_ADDRESSES[i].hi)) {
                continue;
            }
            uint32_t device_address = address - VM_DEVICE_ADDRESSES[i].lo;
            if (vm->devices[i]->read_u8(
                vm->devices[i],
                device_address,
                byte
            ) == 0) {
                device_found = 1;
                break;
            }
        }
        if (device_found != 0) {
            return OK;
        }
        /* Try memory mapped registers */
        if (    (address >= MTIME_ADDRESS)
             && (address < MTIME_ADDRESS + MTIME_BYTE_SIZE)) {
            uint32_t offset = address - MTIME_ADDRESS;
            *byte = (vm->mtime >> (offset * 8)) & 0xff;
            return OK;
        }
        else if (    (address >= MTIMECMP_ADDRESS)
                  && (address < MTIMECMP_ADDRESS + MTIMECMP_BYTE_SIZE)) {
            uint32_t offset = address - MTIMECMP_ADDRESS;
            *byte = (vm->mtime >> (offset * 8)) & 0xff;
            return OK;
        }
        LOG_WARN("Invalid address = 0x%08x", address);
        return ERROR_INVALID_ADDRESS;
    }
    else {
        *byte = page[address & R5_HARDWARE_PAGE_MASK];
        return OK;
    }
}


enum result vm_set_u8(struct vm * vm, uint32_t address, uint8_t byte) {
    if (address == 0x80000000) {
        LOG_WARN("Debug Stdout = 0x%02x (%c)", byte, byte);
        return 0;
    }

    if (vm->tracing) {
        printf("vm_set_u8 0x%08x=0x%02x\n", address, byte);
    }

    uint8_t * page = hardware_pages_get_page(vm->hardware_pages, address);

    if (page == NULL) {
        /* Try devices */
        int device_found = 0;
        unsigned int i;
        for (i = 0; i < vm->num_devices; i++) {
            if (    (address < VM_DEVICE_ADDRESSES[i].lo)
                 || (address >= VM_DEVICE_ADDRESSES[i].hi)) {
                continue;
            }
            uint32_t device_address = address - VM_DEVICE_ADDRESSES[i].lo;
            if (vm->devices[i]->write_u8(
                vm->devices[i],
                device_address,
                byte
            ) == 0) {
                device_found = 1;
                break;
            }
        }
        if (device_found != 0) {
            return OK;
        }
        /* Try memory mapped registers */
        if (    (address >= MTIME_ADDRESS)
             && (address < MTIME_ADDRESS + MTIME_BYTE_SIZE)) {
            uint32_t offset = address - MTIME_ADDRESS;
            uint64_t mask = 0xff;
            mask <<= (offset * 8);
            mask = ~mask;
            vm->mtimecmp &= mask;
            vm->mtime |= ((uint64_t) byte) << (offset * 8);
            return OK;
        }
        else if (    (address >= MTIMECMP_ADDRESS)
                  && (address < MTIMECMP_ADDRESS + MTIMECMP_BYTE_SIZE)) {
            if (vm->tracing) {
                printf("Writing to mtimecmp_address old=%lx ", vm->mtimecmp);
            }
            uint32_t offset = address - MTIMECMP_ADDRESS;
            uint64_t mask = 0xff;
            mask <<= (offset * 8);
            mask = ~mask;
            vm->mtimecmp &= mask;
            vm->mtimecmp |= ((uint64_t) byte) << (offset * 8);
            vm->mtimecmp_written = 1;
            return OK;
        }
        LOG_WARN("Invalid address = 0x%08x", address);
        return ERROR_INVALID_ADDRESS;
    }
    else {
        page[address & R5_HARDWARE_PAGE_MASK] = byte;
        return OK;
    }
}


enum result vm_get_u16(struct vm * vm, uint32_t address, uint16_t * half) {
    uint8_t bytes[2];
    int err;

    if (    ((err = vm_get_u8(vm, address + 0, &bytes[1])) != OK)
         || ((err = vm_get_u8(vm, address + 1, &bytes[0])) != OK)) {
        return err;
    }
    
    *half = bytes[0];
    *half <<= 8;
    *half |= bytes[1];

    return OK;
}


enum result vm_get_u32(struct vm * vm, uint32_t address, uint32_t * word) {
    uint8_t bytes[4];
    int err;

    if (    ((err = vm_get_u8(vm, address + 0, &bytes[3])) != OK)
         || ((err = vm_get_u8(vm, address + 1, &bytes[2])) != OK)
         || ((err = vm_get_u8(vm, address + 2, &bytes[1])) != OK)
         || ((err = vm_get_u8(vm, address + 3, &bytes[0])) != OK)) {
        return err;
    }
    
    *word = bytes[0];
    *word <<= 8;
    *word |= bytes[1];
    *word <<= 8;
    *word |= bytes[2];
    *word <<= 8;
    *word |= bytes[3];

    return OK;
}


enum result vm_set_u16(struct vm * vm, uint32_t address, uint16_t half) {
    int err;

    if (    ((err = vm_set_u8(vm, address + 1, (half >> 8) & 0xff)) != OK)
         || ((err = vm_set_u8(vm, address + 0, (half >> 0) & 0xff)) != OK)) {
        return err;
    }

    return OK;
}


enum result vm_set_u32(struct vm * vm, uint32_t address, uint32_t word) {
    int err;

    if (    ((err = vm_set_u8(vm, address + 3, (word >> 24) & 0xff)) != OK)
         || ((err = vm_set_u8(vm, address + 2, (word >> 16) & 0xff)) != OK)
         || ((err = vm_set_u8(vm, address + 1, (word >> 8) & 0xff)) != OK)
         || ((err = vm_set_u8(vm, address + 0, (word >> 0) & 0xff)) != OK)) {
        return err;
    }

    return OK;
}


enum result vm_set_buf(
    struct vm * vm,
    uint32_t address,
    const uint8_t * buf,
    uint32_t buf_size
) {
    uint32_t i;
    int err;

    for (i = 0; i < buf_size; i++) {
        if ((err = vm_set_u8(vm, address + i, buf[i])) != OK) {
            return err;
        }
    }

    return OK;
}


enum result vm_get_buf(
    struct vm * vm,
    uint32_t address,
    uint8_t * buf,
    uint32_t buf_size
) {
    uint32_t i;
    int err;

    for (i = 0; i < buf_size; i++) {
        if ((err = vm_get_u8(vm, address + i, &(buf[i]))) != OK) {
            return err;
        }
    }

    return OK;
}