#include "debug_device.h"

#include <stdlib.h>

#include "log.h"

struct debug_device * debug_device_create() {
    struct debug_device * debug_device = malloc(sizeof(struct debug_device));
    if (debug_device == NULL) {
        return NULL;
    }

    debug_device->device.delete = (device_delete_f) debug_device_delete;
    debug_device->device.run = (device_run_f) debug_device_run;
    debug_device->device.read_u8 = (device_read_u8_f) debug_device_read_u8;
    debug_device->device.write_u8 = (device_write_u8_f) debug_device_write_u8;

    return debug_device;
}


void debug_device_delete(struct debug_device * debug_device) {
    free(debug_device);
}


enum device_run_result debug_device_run(
    struct debug_device * debug_device,
    struct vm * vm
) {
    return DEVICE_OK;
}


int debug_device_read_u8(
    struct debug_device * debug_device,
    uint32_t address,
    uint8_t * value
) {
    if (address == DEBUG_DEVICE_WRITE_OFFSET) {
        *value = 0;
        return 0;
    }
    else if (    (address >= DEBUG_DEVICE_BUF_OFFSET)
              && (address < DEBUG_DEVICE_BUF_OFFSET + DEBUG_DEVICE_BUF_SIZE)) {
        *value = 0;
        return 0;
    }
    
    return -1;
}


int debug_device_write_u8(
    struct debug_device * debug_device,
    uint32_t address,
    uint8_t value
) {
    if (address == DEBUG_DEVICE_WRITE_OFFSET) {
        if (value == DEBUG_DEVICE_SEND_STRING) {
            LOG_WARN("DEBUG: %s", debug_device->buf);
        }
        else if (value == DEBUG_DEVICE_SEND_UINT8) {
            LOG_WARN("DEBUG: 0x%02x", debug_device->buf[0]);
        }
        else if (value == DEBUG_DEVICE_SEND_UINT32) {
            LOG_WARN("DEBUG 0x%08x", *((uint32_t *) debug_device->buf));
        }
    }
    else if (    (address >= DEBUG_DEVICE_BUF_OFFSET) 
              && (address < DEBUG_DEVICE_BUF_OFFSET + DEBUG_DEVICE_BUF_SIZE)) {
        debug_device->buf[address - DEBUG_DEVICE_BUF_OFFSET] = value;
    }
    else {
        return -1;
    }
    return 0;
}