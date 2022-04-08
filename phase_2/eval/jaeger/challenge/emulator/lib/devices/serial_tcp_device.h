#ifndef serial_tcp_device_HEADER
#define serial_tcp_device_HEADER


/*
Provides a bridge between a TCP connection, and an IC which buffers serial data,
and provides that data through data lines.

0x0000: uint8_t STATUS
0x0001: uint8_t READ_SIZE
0x0002: uint8_t WRITE_SIZE
0x0010: uint8_t READ_BUFFER[16]
0x0020: uint8_t WRITE_BUFFER[16]

Status Bit Mask:
0: READ_DATA_READY
1: READ_DATA_COMPLETE
2: WRITE_DATA_READY
3: WRITE_DATA_COMPLETE

Receiving data:

When READ_DATA_READY is set to 0, the CM reads up to READ_BUFFER_SIZE bytes
into READ_BUFFER. The byte at READ_SIZE is set to the number of bytes placed in
READ_BUFFER, the READ_DATA_COMPLETE bit is set to 0, and then the
READ_DATA_READY bit is set to 1.

The application will consume the data in READ_BUFFER.
When complete, the application will set READ_DATA_COMPLETE to 1, and then set
READ_DATA_READY to 0.

Sending data:

When WRITE_DATA_READY is set to 1, the CM will consume the data in WRITE_BUFFER.
When the data has been consumed and sent, the CM will set WRITE_DATA_COMPLETE to
1, and then set WRITE_DATA_READY to 0.

When WRITE_DATA_READY is set to 0, the application will place up to
WRITE_BUFFER_SIZE bytes in WRITE_BUFFER. WRITE_SIZE will be set to the number of
bytes placed in WRITE_BUFFER. The WRITE_DATA_COMPLETE bit will be set to 0, and
then the WRITE_DATA_READY bit will be set to 1.
*/

#include "device.h"

#include <stdlib.h>


#define READ_DATA_READY_BIT 1
#define READ_DATA_COMPLETE_BIT 2
#define WRITE_DATA_READY_BIT 4
#define WRITE_DATA_COMPLETE_BIT 8

#define GET_READ_DATA_READY_BIT(STD) \
    (((STD->status_byte) & READ_DATA_READY_BIT) > 0 ? 1 : 0)
#define GET_READ_DATA_COMPLETE_BIT(STD) \
    (((STD->status_byte) & READ_DATA_COMPLETE_BIT) > 0 ? 1 : 0)
#define GET_WRITE_DATA_READY_BIT(STD) \
    (((STD->status_byte) & WRITE_DATA_READY_BIT) > 0 ? 1 : 0)
#define GET_WRITE_DATA_COMPLETE_BIT(STD) \
    (((STD->status_byte) & WRITE_DATA_COMPLETE_BIT) > 0 ? 1 : 0)

#define SET_READ_DATA_READY_BIT(STD, VALUE) \
    do { \
        STD->status_byte = (STD->status_byte & (~READ_DATA_READY_BIT)) | VALUE; \
    } while (0);
#define SET_READ_DATA_COMPLETE_BIT(STD, VALUE) \
    do { \
        STD->status_byte = \
            (STD->status_byte & (~READ_DATA_COMPLETE_BIT)) | VALUE << 1; \
    } while (0);
#define SET_WRITE_DATA_READY_BIT(STD, VALUE) \
    do { \
        STD->status_byte = \
            (STD->status_byte & (~WRITE_DATA_READY_BIT)) | VALUE << 2; \
    } while (0);
#define SET_WRITE_DATA_COMPLETE_BIT(STD, VALUE) \
    do { \
        STD->status_byte = \
            (STD->status_byte & (~WRITE_DATA_COMPLETE_BIT)) | VALUE << 3; \
    } while (0);

#define STATUS_BYTE_OFFSET 0
#define READ_SIZE_OFFSET 1
#define WRITE_SIZE_OFFSET 2
#define READ_BUFFER_OFFSET 0x100
#define WRITE_BUFFER_OFFSET 0x200
#define READ_BUFFER_SIZE 0x80
#define WRITE_BUFFER_SIZE 0x80

struct serial_tcp_device {
    struct device device;
    int sockfd;

    uint8_t status_byte;
    uint8_t read_size;
    uint8_t write_size;
    uint8_t read_buffer[READ_BUFFER_SIZE];
    uint8_t write_buffer[WRITE_BUFFER_SIZE];
};

struct serial_tcp_device * serial_tcp_device_accept(
    const char * port
);

void serial_tcp_device_delete(struct serial_tcp_device * serial_tcp_device);

enum device_run_result serial_tcp_device_run(
    struct serial_tcp_device * serial_tcp_device,
    struct vm * vm
);

int serial_tcp_device_read_u8(
    struct serial_tcp_device * serial_tcp_device,
    uint32_t address,
    uint8_t * value
);

int serial_tcp_device_write_u8(
    struct serial_tcp_device * serial_tcp_device,
    uint32_t address,
    uint8_t value
);

#endif