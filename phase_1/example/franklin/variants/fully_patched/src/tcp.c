#include "tcp.h"

extern stream *st;

tcp *parse_tcp_header( )
{
    tcp *tcph = NULL;

    if ( !st ) {
        return NULL;
    }

    tcph = malloc( sizeof(tcp ) );

    if ( !tcph ) {
        write_data(st, "[ERROR] Failed to allocate buffer\n");
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&tcph->sport, 16 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&tcph->dport, 16 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&tcph->sequence_no, 32 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&tcph->ack_no, 32 ) ) {
        return NULL;
    } 

    if ( !get_bits( st, (uint8_t*)&tcph->data_offset, 4 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&tcph->reserved, 3 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&tcph->flags, 9 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&tcph->window_sz, 16 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&tcph->checksum, 16 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&tcph->urgent_ptr, 16 ) ) {
        return NULL;
    }

    write_data(st, "TCP\n");
    write_data(st, "\tSource Port: %d\n", htons(tcph->sport));
    write_data(st, "\tDest Port: %d\n", htons(tcph->dport));
    write_data(st, "\tSeq No: %x\n", htonl(tcph->sequence_no));
    write_data(st, "\tAck No: %x\n", htonl(tcph->ack_no));
    write_data(st, "\tData Offset: %d\n", tcph->data_offset );

    
    write_data(st, "\tFlags: %x\n", tcph->flags );
    write_data(st, "\tWindow Size: %x\n", htons(tcph->window_sz));
    write_data(st, "\tChecksum: %x\n", htons(tcph->checksum));
    write_data(st, "\tUrgent Ptr: %d\n", htons(tcph->urgent_ptr) );

    
    

    return tcph;
}