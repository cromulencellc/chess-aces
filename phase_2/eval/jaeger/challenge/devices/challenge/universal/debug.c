#include "../debug.h"

#include "platform/stdlib.h"
#include "platform/platform.h"

/* Pointers to the debug device */

volatile uint8_t * DEBUG_DEVICE_WRITE =
    (uint8_t *) (DEVICE_3_ADDRESS + DEBUG_DEVICE_WRITE_OFFSET);
volatile uint8_t * DEBUG_DEVICE_BUF =
    (uint8_t *) (DEVICE_3_ADDRESS + DEBUG_DEVICE_BUF_OFFSET);





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