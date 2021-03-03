#include "igmp.h"

extern stream *st;

igmp *parse_igmp( )
{
    igmp *pigmp;

    if ( st == NULL ) {
        return NULL;
    }

    pigmp = malloc( sizeof(igmp) );

    if ( !pigmp ) {
        return NULL;
    }

    memset( pigmp, 0, sizeof( igmp ) );

    if (!get_bits( st, (uint8_t*)&pigmp->type, 8 ) ) {
        free( pigmp );
        return NULL;
    }

    if (!get_bits( st, (uint8_t*)&pigmp->max_resp_time, 8 ) ) {
        free( pigmp );
        return NULL;
    }

    if (!get_bits( st, (uint8_t*)&pigmp->checksum, 16 ) ) {
        free( pigmp );
        return NULL;
    }

    if (!get_bits( st, (uint8_t*)&pigmp->group_addr, 32 ) ) {
        free( pigmp );
        return NULL;
    }

    write_data(st, "IGMP: \n");
    write_data(st, "\tType: ");

    switch ( pigmp->type ) {
        case 0x11:
            write_data(st, "Membership Query\n");
            break;
        case 0x12:
            write_data(st, "IGMPv1 Membership Report\n");
            break;
        case 0x16:
            write_data(st, "IGMPv2 Membership Report\n");
            break;
        case 0x22:
            write_data(st, "IGMPv3 Membership Report\n");
            break;
        case 0x17:
            write_data(st, "Leave Group\n");
            break;
        default:  
            write_data(st, "Unknown IGMP message type\n");
            break;
    }

    write_data(st, "\tMax Response Time: %d\n", pigmp->max_resp_time );
    write_data(st, "\tGroup Address: ");

    print_ipv4_addr( pigmp->group_addr );
    write_data(st, "\n");

    return pigmp;
}