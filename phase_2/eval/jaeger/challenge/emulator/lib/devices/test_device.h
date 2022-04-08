#ifndef test_device_HEADER
#define test_device_HEADER

/*
* This is a simplistic device for testing.
*
* It allows a device to quit the emulator in a controlled manner if a test
* fails.
*/

#include "bus/bus.h"

#include "device.h"

#include <stdlib.h>


struct test_device {
    struct device device;
};

struct test_device * test_device_create();
void test_device_delete(struct test_device * test_device);

enum device_run_result test_device_run(
    struct test_device * test_device,
    struct vm * vm
);

int test_device_read_u8(
    struct test_device * test_device,
    uint32_t address,
    uint8_t * value
);

int test_device_write_u8(
    struct test_device * test_device,
    uint32_t address,
    uint8_t value
);

#endif