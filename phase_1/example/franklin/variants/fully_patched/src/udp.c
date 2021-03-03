#include "udp.h"

extern stream *st;

udp *parse_udp_header( )
{
    udp *uh;
    int length;

    if ( !st ) {
        return NULL;
    }

    uh = malloc( sizeof(udp) );

    if ( !uh ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&uh->sport, 16 ) ) {
        free( uh );
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&uh->dport, 16 ) ) {
        free( uh );
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&uh->length, 16 ) ) {
        free( uh );
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&uh->checksum, 16 ) ) {
        free( uh );
        return NULL;
    }

    write_data(st, "UDP\n");
    write_data(st, "\tSource prot: %d\n", htons(uh->sport));
    write_data(st, "\tDest port: %d\n", htons(uh->dport));
    write_data(st, "\tLength: %d\n", htons(uh->length));
    write_data(st, "\tChecksum: %x\n", htons(uh->checksum));


    
    length = htons(uh->length) - 8;

    uh->data = malloc( length );

    if (!uh->data) {
        free(uh);
        return NULL;
    }

    if ( !get_bits( st, uh->data, length * 8) ) {
        write_data(st, "[ERROR] Not enough datas\n");
        free( uh->data );
        free( uh );
        return NULL;
    }

    write_data(st, "\tDATA: ");
    for ( int i = 0; i < length; i++ ) {
        write_data(st, "%.2x ", uh->data[i]);
    }

    write_data(st, "\n");
    return uh;
}