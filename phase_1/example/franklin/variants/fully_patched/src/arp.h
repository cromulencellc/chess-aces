#ifndef __ARP__
#define __ARP__

#include "stream.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct arp {
    uint16_t hrd;
    uint16_t prot;
    uint8_t hln;
    uint8_t pln;
    uint16_t opcode;

    uint8_t *sender_pa;
    uint8_t *sender_ha;
    uint8_t *dest_pa;
    uint8_t *dest_ha;
} arp;

arp* parse_arp( );

#endif