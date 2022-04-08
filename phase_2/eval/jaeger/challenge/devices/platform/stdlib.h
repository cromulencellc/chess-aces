#ifndef stdlib_HEADER
#define stdlib_HEADER

#include "platform.h"

int memcmp(const void * lhs, const void * rhs, uint32_t len);
void * memcpy(void * dst, const void * src, uint32_t len);
void * memmove(void * dst, const void * src, uint32_t len);
void * memset(void * buf, uint8_t c, uint32_t len);
char * strcpy(char * dst, const char * src);
unsigned int strlen(const char * s);
void puts(const char * s);

void redirect_puts_to_debug_device(void * debug_device_address);

#endif