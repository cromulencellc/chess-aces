#include "test_device.h"

#include <stdlib.h>

#include "log.h"

struct test_device * test_device_create() {
    struct test_device * test_device = malloc(sizeof(struct test_device));
    if (test_device == NULL) {
        return NULL;
    }

    test_device->device.delete = (device_delete_f) test_device_delete;
    test_device->device.run = (device_run_f) test_device_run;
    test_device->device.read_u8 = (device_read_u8_f) test_device_read_u8;
    test_device->device.write_u8 = (device_write_u8_f) test_device_write_u8;

    return test_device;
}


void test_device_delete(struct test_device * test_device) {
    free(test_device);
}


enum device_run_result test_device_run(
    struct test_device * test_device,
    struct vm * vm
) {
    return DEVICE_OK;
}


int test_device_read_u8(
    struct test_device * test_device,
    uint32_t address,
    uint8_t * value
) {
    *value = 0;
    return 0;
}


int test_device_write_u8(
    struct test_device * test_device,
    uint32_t address,
    uint8_t value
) {
    if (address == 0) {
        LOG_ERROR("TEST DEVICE FAIL TEST 0x%02X", value);
        fflush(stdout);
        exit(1);
    }
    else if (address == 1) {
        LOG_SUCCESS("TEST DEVICE PASS TEST 0x%02X", value);
    }
    return 0;
}