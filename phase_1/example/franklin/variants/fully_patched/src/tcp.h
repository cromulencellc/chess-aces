#ifndef __TCP__
#define __TCP__

#include "stream.h"
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#pragma pack(push, 1)
typedef struct tcp {
    uint16_t sport;
    uint16_t dport;
    uint32_t sequence_no;
    uint32_t ack_no;
    uint8_t data_offset;    
    uint8_t reserved; 
    uint16_t flags; 
    uint16_t window_sz;
    uint16_t checksum;
    uint16_t urgent_ptr;
    uint8_t options[];
} tcp;

#pragma pack(pop)

tcp *parse_tcp_header( );

#endif