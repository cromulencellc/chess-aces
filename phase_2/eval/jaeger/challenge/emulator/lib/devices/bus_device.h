#ifndef bus_device_HEADER
#define bus_device_HEADER

#include "bus/bus.h"

/*
Provides a memory-mapped interface to a bus.

0x0000: uint8_t STATUS
0x0001: uint8_t READ_SIZE
0x0010: uint8_t READ_BUFFER[16]
0x0002: uint8_t WRITE_SIZE
0x0020: uint8_t WRITE_BUFFER[16]

Status Bit Mask:
0: READ_DATA_READY
1: READ_DATA_COMPLETE
2: WRITE_DATA_READY
3: WRITE_DATA_COMPLETE

Receiving data:

When READ_DATA_READY is set to 0, the IC reads up to READ_BUFFER_SIZE bytes
into READ_BUFFER. The byte at READ_SIZE is set to the number of bytes placed in
READ_BUFFER, the READ_DATA_COMPLETE bit is set to 0, and then the
READ_DATA_READY bit is set to 1.

The application will consume the data in READ_BUFFER.
When complete, the application will set READ_DATA_COMPLETE to 1, and then set
READ_DATA_READY to 0.

Sending data:

When WRITE_DATA_READY is set to 1, the IC will consume the data in WRITE_BUFFER.
When the data has been consumed and sent, the IC will set WRITE_DATA_COMPLETE to
1, and then set WRITE_DATA_READY to 0.

When WRITE_DATA_READY is set to 0, the application will place up to
WRITE_BUFFER_SIZE bytes in WRITE_BUFFER. WRITE_SIZE will be set to the number of
bytes placed in WRITE_BUFFER. The WRITE_DATA_COMPLETE bit will be set to 0, and
then the WRITE_DATA_READY bit will be set to 1.
*/

#include "device.h"

#include <stdlib.h>


#define BUS_DEVICE_READ_DATA_READY_BIT 1
#define BUS_DEVICE_READ_DATA_COMPLETE_BIT 2
#define BUS_DEVICE_WRITE_DATA_READY_BIT 4
#define BUS_DEVICE_WRITE_DATA_COMPLETE_BIT 8

#define BUS_DEVICE_READ_DATA_READY_SHIFT 0
#define BUS_DEVICE_READ_DATA_COMPLETE_SHIFT 1
#define BUS_DEVICE_WRITE_DATA_READY_SHIFT 2
#define BUS_DEVICE_WRITE_DATA_COMPLETE_SHIFT 3

#define BUS_DEVICE_GET_READ_DATA_READY_BIT(BD) \
    (((BD->status_byte) & BUS_DEVICE_READ_DATA_READY_BIT) \
        >> BUS_DEVICE_READ_DATA_READY_SHIFT)

#define BUS_DEVICE_GET_READ_DATA_COMPLETE_BIT(BD) \
    (((BD->status_byte) & BUS_DEVICE_READ_DATA_COMPLETE_BIT) \
        >> BUS_DEVICE_READ_DATA_COMPLETE_SHIFT)

#define BUS_DEVICE_GET_WRITE_DATA_READY_BIT(BD) \
    (((BD->status_byte) & BUS_DEVICE_WRITE_DATA_READY_BIT) \
        >> BUS_DEVICE_WRITE_DATA_READY_SHIFT)

#define BUS_DEVICE_GET_WRITE_DATA_COMPLETE_BIT(BD) \
    (((BD->status_byte) & BUS_DEVICE_WRITE_DATA_COMPLETE_BIT) \
        >> BUS_DEVICE_WRITE_DATA_COMPLETE_SHIFT)

#define BUS_DEVICE_SET_READ_DATA_READY_BIT(BD, VALUE) \
    do { \
        BD->status_byte = \
            (BD->status_byte & (~BUS_DEVICE_READ_DATA_READY_BIT)) | VALUE; \
    } while (0);
#define BUS_DEVICE_SET_READ_DATA_COMPLETE_BIT(BD, VALUE) \
    do { \
        BD->status_byte = \
            (BD->status_byte & (~BUS_DEVICE_READ_DATA_COMPLETE_BIT)) | VALUE << 1; \
    } while (0);
#define BUS_DEVICE_SET_WRITE_DATA_READY_BIT(BD, VALUE) \
    do { \
        BD->status_byte = \
            (BD->status_byte & (~BUS_DEVICE_WRITE_DATA_READY_BIT)) | VALUE << 2; \
    } while (0);
#define BUS_DEVICE_SET_WRITE_DATA_COMPLETE_BIT(BD, VALUE) \
    do { \
        BD->status_byte = \
            (BD->status_byte & (~BUS_DEVICE_WRITE_DATA_COMPLETE_BIT)) | VALUE << 3; \
    } while (0);

#define BUS_DEVICE_STATUS_BYTE_OFFSET 0
#define BUS_DEVICE_READ_SIZE_OFFSET 1
#define BUS_DEVICE_WRITE_SIZE_OFFSET 2
#define BUS_DEVICE_READ_BUFFER_OFFSET 0x100
#define BUS_DEVICE_WRITE_BUFFER_OFFSET 0x200
#define BUS_DEVICE_READ_BUFFER_SIZE 16
#define BUS_DEVICE_WRITE_BUFFER_SIZE 16

struct bus_device {
    struct device device;
    struct bus_connection * bus_connection;
    uint8_t status_byte;
    uint8_t read_size;
    uint8_t write_size;
    uint8_t read_buffer[BUS_DEVICE_READ_BUFFER_SIZE];
    uint8_t write_buffer[BUS_DEVICE_WRITE_BUFFER_SIZE];
};

struct bus_device * bus_device_create(struct bus_connection * bus_connection);

void bus_device_delete(struct bus_device * bus_device);

enum device_run_result bus_device_run(
    struct bus_device * bus_device,
    struct vm * vm
);

int bus_device_read_u8(
    struct bus_device * bus_device,
    uint32_t address,
    uint8_t * value
);

int bus_device_write_u8(
    struct bus_device * bus_device,
    uint32_t address,
    uint8_t value
);

#endif