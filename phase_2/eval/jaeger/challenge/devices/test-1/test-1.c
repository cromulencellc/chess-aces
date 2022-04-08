#include "test-1.h"

#include "platform/stdlib.h"


/* Pointers to the memory-mapped bus controller */
volatile uint8_t * BUS_STATUS_PTR =
    (uint8_t *) (DEVICE_0_ADDRESS + STATUS_BYTE_OFFSET);
volatile uint8_t * BUS_READ_SIZE_PTR =
    (uint8_t *) (DEVICE_0_ADDRESS + READ_SIZE_OFFSET);
volatile uint8_t * BUS_WRITE_SIZE_PTR =
    (uint8_t *) (DEVICE_0_ADDRESS + WRITE_SIZE_OFFSET);

volatile uint8_t * BUS_READ_BUFFER =
    (uint8_t *) (DEVICE_0_ADDRESS + READ_BUFFER_OFFSET);
volatile uint8_t * BUS_WRITE_BUFFER =
    (uint8_t *) (DEVICE_0_ADDRESS + WRITE_BUFFER_OFFSET);

/* Pointers to the debug device */

volatile uint8_t * DEBUG_DEVICE_WRITE =
    (uint8_t *) (DEVICE_1_ADDRESS + DEBUG_DEVICE_WRITE_OFFSET);
volatile uint8_t * DEBUG_DEVICE_BUF =
    (uint8_t *) (DEVICE_1_ADDRESS + DEBUG_DEVICE_BUF_OFFSET);



void debug_string(const char * s) {
    strcpy((char *) DEBUG_DEVICE_BUF, s);
    *DEBUG_DEVICE_WRITE = DEBUG_DEVICE_SEND_STRING;
}

void debug_uint8(uint8_t u8) {
    *DEBUG_DEVICE_BUF = u8;
    *DEBUG_DEVICE_WRITE = DEBUG_DEVICE_SEND_UINT8;
}

void debug_uint32(uint32_t u32) {
    *((uint32_t *) DEBUG_DEVICE_BUF) = u32;
    *DEBUG_DEVICE_WRITE = DEBUG_DEVICE_SEND_UINT32;
}