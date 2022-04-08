#include "platform.h"


uint8_t * PUTS_DEBUG_ADDRESS = 0;

void redirect_puts_to_debug_device(void * debug_device_address) {
    PUTS_DEBUG_ADDRESS = (uint8_t *) debug_device_address;
}


int memcmp(const void * lhs, const void * rhs, uint32_t len) {
    const uint32_t * lhs32 = (uint32_t *) lhs;
    const uint32_t * rhs32 = (uint32_t *) rhs;

    while (len > 4) {
        if (*lhs32 < *rhs32) {
            return -1;
        }
        else if (*lhs32 > *rhs32) {
            return 1;
        }
        else {
            len -= 4;
            lhs32++;
            rhs32++;
        }
    }

    const uint8_t * lhs8 = (uint8_t *) lhs32;
    const uint8_t * rhs8 = (uint8_t *) rhs32;

    while (len > 0) {
        if (*lhs8 < *rhs8) {
            return -1;
        }
        else if (*lhs8 > *rhs8) {
            return 1;
        }
        else {
            len--;
            lhs8++;
            rhs8++;
        }
    }

    return 0;
}


void * memset(void * buf, uint8_t c, uint32_t len) {
    uint32_t * buf32 = (uint32_t *) buf;
    uint32_t c32 = c;
    c32 <<= 8;
    c32 |= c;
    c32 <<= 8;
    c32 |= c;
    c32 <<= 8;
    c32 |= c;
    
    while (len > 4) {
        *buf32 = c32;
        buf32++;
        len -= 4;
    }

    uint8_t * buf8 = (uint8_t *) buf32;
    while (len > 0) {
        *buf8 = c;
        buf8++;
        len--;
    }

    return buf;
}


void * memcpy(void * dst, const void * src, uint32_t len) {
    uint32_t * dst32 = (uint32_t *) dst;
    const uint32_t * src32 = (const uint32_t *) src;

    while (len > 4) {
        *dst32 = *src32;
        dst32++;
        src32++;
        len -= 4;
    }

    uint8_t * dst8 = (uint8_t *) dst32;
    uint8_t * src8 = (uint8_t *) src32;

    while (len > 0) {
        *dst8 = *src8;
        dst8++;
        src8++;
        len--;
    }

    return dst;
}


void * memmove(void * dst, const void * src, uint32_t len) {
    uint8_t * dst8 = (uint8_t *) src;
    const uint8_t * src8 = (const uint8_t *) src;

    while (len--) {
        *dst8 = *src8;
        dst8++;
        src8++;
    }

    return dst;
}


char * strcpy(char * dst, const char * src) {
    char * saved_dst = dst;
    while (*src != 0) {
        *dst = *src;
        dst++;
        src++;
    }
    *dst = 0;
    return saved_dst;
}


unsigned int strlen(const char * s) {
    unsigned int len = 0;
    while (*s) {
        len++;
        s++;
    }
    return len;
}


void puts(const char * s) {
    if (PUTS_DEBUG_ADDRESS == 0) {
        unsigned int len = strlen(s);
        unsigned int i;
        for (i = 0; i < len; i++) {
            *((uint8_t *) 0x80000000) = s[i];
        }
    }
    else {
        strcpy(&PUTS_DEBUG_ADDRESS[DEBUG_DEVICE_BUF_OFFSET], s);
        *PUTS_DEBUG_ADDRESS = DEBUG_DEVICE_SEND_STRING;
    }
}