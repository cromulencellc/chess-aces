#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include "stream.h"
#include "arp.h"
#include "ipheader.h"
#include <stdint.h>

#define ETH_TYPE_IPV4 0x0008
#define ETH_TYPE_IPV6 0xdd86
#define ETH_TYPE_ARP 0x0608
#define ETH_TYPE_LOOP 0x0090

#pragma pack(push, 1)
typedef struct eth {
    uint8_t dest[6];
    uint8_t src[6];
    uint16_t type;

    iphv6 *ip6;
    iphv4 *ip4;

    arp* parp;


} eth;
#pragma pack(pop)


eth *parse_ethernet_header( );

#endif