#include "ethernet.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern stream *st;

int parse_loop( )
{
    uint16_t skipcount;
    uint16_t function;
    uint16_t receiptno;
    uint8_t byte;

    if ( !st ) {
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&skipcount, 16 ) ) {
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&function, 16 ) ) {
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&receiptno, 16 ) ) {
        return 0;
    }

    write_data(st, "LOOP:\n");
    write_data(st, "\tSkip count; %d\n", skipcount);
    write_data(st, "\tFunction: %d\n", function);
    write_data(st, "\tReceipt Number: %d\n", receiptno);

    write_data(st, "\tData: ");

    while ( get_bits( st, &byte, 8) ) {

        write_data(st, "%.2x ", byte );
    }

    write_data(st, "\n");

    return 1;
}


eth *parse_ethernet_header( )
{
    eth * ethernet_header = NULL;

    int a,b,c,d,e,f;

    if ( st == NULL ) {
        return NULL;
    }

    ethernet_header = malloc( 14 );

    if ( !ethernet_header ) {
        write_data(st, "[ERROR] Failed to allocate eth header\n");
        return NULL;
    }

    memset( ethernet_header, 0, 14 );

    if ( !get_bits( st, ethernet_header->dest, 48 ) ) {
        free( ethernet_header );
        return NULL;
    }

    if ( !get_bits( st, ethernet_header->src, 48) ) {
        free( ethernet_header );
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&ethernet_header->type, 16 ) ) {
        free( ethernet_header );
        return NULL;
    }

    a = ethernet_header->dest[0];
    b = ethernet_header->dest[1];
    c = ethernet_header->dest[2];
    d = ethernet_header->dest[3];
    e = ethernet_header->dest[4];
    f = ethernet_header->dest[5];

    write_data(st, "Ethernet\n");
    write_data(st, "\tDest: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", a,b,c,d,e,f);

    a = ethernet_header->src[0];
    b = ethernet_header->src[1];
    c = ethernet_header->src[2];
    d = ethernet_header->src[3];
    e = ethernet_header->src[4];
    f = ethernet_header->src[5];

    write_data(st, "\tSrc: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", a,b,c,d,e,f);

    if ( ethernet_header->type == ETH_TYPE_IPV4) {
        write_data(st, "\tType: IPv4\n");

        ethernet_header->ip4 = parse_ipv4_header( st );
        if ( ethernet_header->ip4 == NULL ) {
            write_data(st, "[ERROR] Failed to parse IPv4 header\n");
        }

    } else if ( ethernet_header->type == ETH_TYPE_IPV6 ) {
        write_data(st, "\tType: IPV6\n");

        ethernet_header->ip6 = parse_ipv6_header( st );
        if ( ethernet_header->ip4 == NULL ) {
            write_data(st, "[ERROR] Failed to parse IPv6 header\n");
        }

    } else if ( ethernet_header->type == ETH_TYPE_ARP ) {
        write_data(st, "\tType: ARP\n");

        ethernet_header->parp = parse_arp( st );

        if ( ethernet_header->parp == NULL ) {
            write_data(st, "[ERROR] Failed to parse ARP\n");
        }
    } else if ( ethernet_header->type == ETH_TYPE_LOOP ) {
        write_data(st, "\tType: LOOP\n");

        if ( !parse_loop( ) ) {
            write_data(st, "[ERROR] Failed to parse LOOP\n");
        }

    } else {
        write_data(st, "\tType: Unknown (0x%.4x)\n", ethernet_header->type);
    }

    write_data(st, "Done\n");
    return ethernet_header;
}