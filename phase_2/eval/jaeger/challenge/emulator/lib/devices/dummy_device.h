#ifndef dummy_device_HEADER
#define dummy_device_HEADER

/*
* This is a simplistic device for testing.
*
* It allows a device to quit the emulator in a controlled manner if a test
* fails.
*/

#include "bus/bus.h"

#include "device.h"

#include <stdlib.h>


struct dummy_device {
    struct device device;
};

struct dummy_device * dummy_device_create();
void dummy_device_delete(struct dummy_device * dummy_device);

enum device_run_result dummy_device_run(
    struct dummy_device * dummy_device,
    struct vm * vm
);

int dummy_device_read_u8(
    struct dummy_device * dummy_device,
    uint32_t address,
    uint8_t * value
);

int dummy_device_write_u8(
    struct dummy_device * dummy_device,
    uint32_t address,
    uint8_t value
);

#endif