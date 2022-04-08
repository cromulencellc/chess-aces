#include "dummy_device.h"

#include <stdlib.h>

#include "log.h"

struct dummy_device * dummy_device_create() {
    struct dummy_device * dummy_device = malloc(sizeof(struct dummy_device));
    if (dummy_device == NULL) {
        return NULL;
    }

    dummy_device->device.delete = (device_delete_f) dummy_device_delete;
    dummy_device->device.run = (device_run_f) dummy_device_run;
    dummy_device->device.read_u8 = (device_read_u8_f) dummy_device_read_u8;
    dummy_device->device.write_u8 = (device_write_u8_f) dummy_device_write_u8;

    return dummy_device;
}


void dummy_device_delete(struct dummy_device * dummy_device) {
    free(dummy_device);
}


enum device_run_result dummy_device_run(
    struct dummy_device * dummy_device,
    struct vm * vm
) {
    return DEVICE_OK;
}


int dummy_device_read_u8(
    struct dummy_device * dummy_device,
    uint32_t address,
    uint8_t * value
) {
    *value = 0;
    return 1;
}


int dummy_device_write_u8(
    struct dummy_device * dummy_device,
    uint32_t address,
    uint8_t value
) {
    return 1;
}