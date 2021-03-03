#ifndef __IGMP__
#define __IGMP__

#include "stream.h"
#include "ipheader.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct igmp {
    uint8_t type;
    uint8_t max_resp_time;
    uint16_t checksum;
    uint32_t group_addr;
} igmp;

igmp *parse_igmp( );

#endif