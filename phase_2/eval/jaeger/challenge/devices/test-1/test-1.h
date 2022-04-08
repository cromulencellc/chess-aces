#ifndef test_1_HEADER
#define test_1_HEADER

#include "platform/platform.h"

void debug_string(const char * s);
void debug_uint8(uint8_t u8);
void debug_uint32(uint32_t u32);


/* Pointers to the memory-mapped bus controller */
extern volatile uint8_t * BUS_STATUS_PTR;
extern volatile uint8_t * BUS_READ_SIZE_PTR;
extern volatile uint8_t * BUS_WRITE_SIZE_PTR;

extern volatile uint8_t * BUS_READ_BUFFER;
extern volatile uint8_t * BUS_WRITE_BUFFER;

/* Pointers to the debug device */
extern volatile uint8_t * DEBUG_DEVICE_WRITE;
extern volatile uint8_t * DEBUG_DEVICE_BUF;

#endif