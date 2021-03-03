#ifndef __ICMP__
#define __ICMP__

#include "stream.h"
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#pragma pack(push, 1 )
typedef struct icmp {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;

    uint8_t *data;

} icmp;
#pragma pack(pop)

typedef struct icmpv6 {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
} icmpv6;

int parse_icmpv6( uint16_t length);
int parse_icmpv4( );

#endif