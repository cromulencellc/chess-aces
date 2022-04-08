#ifndef debug_device_HEADER
#define debug_device_HEADER

#include "bus/bus.h"

#include "device.h"

#include <stdlib.h>

#define DEBUG_DEVICE_SEND_STRING 1
#define DEBUG_DEVICE_SEND_UINT8 2
#define DEBUG_DEVICE_SEND_UINT32 3

#define DEBUG_DEVICE_WRITE_OFFSET 0
#define DEBUG_DEVICE_BUF_OFFSET 0x100
#define DEBUG_DEVICE_BUF_SIZE 0x100


struct debug_device {
    struct device device;
    uint8_t buf[DEBUG_DEVICE_BUF_SIZE];
};

struct debug_device * debug_device_create();
void debug_device_delete(struct debug_device * debug_device);

enum device_run_result debug_device_run(
    struct debug_device * debug_device,
    struct vm * vm
);

int debug_device_read_u8(
    struct debug_device * debug_device,
    uint32_t address,
    uint8_t * value
);

int debug_device_write_u8(
    struct debug_device * debug_device,
    uint32_t address,
    uint8_t value
);

#endif