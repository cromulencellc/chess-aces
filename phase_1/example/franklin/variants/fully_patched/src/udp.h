#ifndef __UDP__
#define __UDP__

#include "stream.h"
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#pragma pack(push, 1)
typedef struct udp {
    uint16_t sport;
    uint16_t dport;
    uint16_t length;
    uint16_t checksum;
    uint8_t *data;
} udp;

#pragma pack(pop)

udp *parse_udp_header( );

#endif