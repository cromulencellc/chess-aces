#ifndef __IPH_H__
#define __IPH_H__

#include "stream.h"
#include "tcp.h"
#include "udp.h"
#include "icmp.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#pragma pack(push, 1)
typedef struct iphv6 {
    uint8_t version;
    uint8_t traffic_class;
    uint32_t flow_label;
    uint16_t payload_length;
    uint8_t next_header;
    uint8_t hop_limit;
    uint8_t source[16];
    uint8_t dest[16];

    tcp *ptcp;
} iphv6;

typedef struct iphv4 {
    uint8_t ihl;
    uint8_t version;
    uint8_t dscp;
    uint8_t ecn;
    uint16_t length;
    uint16_t id;
    uint8_t flags;
    uint16_t offset;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    uint32_t source;
    uint32_t dest;

    tcp *ptcp;
    iphv6 *ipv6;
    udp *pudp;
    
    uint8_t options[];      
} iphv4;



#pragma pack(pop)

iphv4 *parse_ipv4_header( );
iphv6 *parse_ipv6_header( );
void print_ipv4_addr ( uint32_t addr );
void print_ipv6_addr( char *addr );

#endif