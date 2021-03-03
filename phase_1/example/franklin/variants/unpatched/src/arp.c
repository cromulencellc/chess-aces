#include "arp.h"

extern stream *st;

arp * parse_arp( ) 
{
    arp *pkt;

    if ( st == NULL ) {
        return NULL;
    }

    pkt = malloc( sizeof( arp ));

    if ( !pkt ) {
        write_data(st, "[ERROR] Failed to allocate block\n");
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&pkt->hrd, 16) ) {
        free( pkt );
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&pkt->prot, 16) ) {
        free( pkt );
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&pkt->hln, 8) ) {
        free( pkt );
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&pkt->pln, 8) ) {
        free( pkt );
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&pkt->opcode, 16) ) {
        free( pkt );
        return NULL;
    }

    pkt->sender_pa = malloc( pkt->pln );

    if ( !pkt->sender_pa ) {
        free(pkt);
        return NULL;
    }
    
    pkt->sender_ha = malloc( pkt->hln );

    if ( !pkt->sender_ha ) {
        free(pkt->sender_pa);
        free(pkt);
        return NULL;
    }

    pkt->dest_pa = malloc( pkt->pln );

    if ( !pkt->dest_pa ) {
        free(pkt->sender_pa);
        free(pkt->sender_ha);
        free(pkt);
        return NULL;
    }

    pkt->dest_ha = malloc( pkt->hln );

    if ( !pkt->dest_ha ) {
        free(pkt->sender_pa);
        free(pkt->sender_ha);
        free(pkt->dest_pa);
        free(pkt);
        return NULL;
    }

    if (!get_bits( st, pkt->sender_ha, pkt->hln*8)) {
        free(pkt->sender_pa);
        free(pkt->sender_ha);
        free(pkt->dest_pa);
        free(pkt);
        return NULL;
    }

    if (!get_bits( st, pkt->sender_pa, pkt->pln*8)) {
        free(pkt->sender_pa);
        free(pkt->sender_ha);
        free(pkt->dest_pa);
        free(pkt);
        return NULL;
    }

    if (!get_bits( st, pkt->dest_ha, pkt->hln * 8)) {
        free(pkt->sender_pa);
        free(pkt->sender_ha);
        free(pkt->dest_pa);
        free(pkt);
        return NULL;
    }

    if (!get_bits( st, pkt->dest_pa, pkt->pln * 8 )) {
        free(pkt->sender_pa);
        free(pkt->sender_ha);
        free(pkt->dest_pa);
        free(pkt);
        return NULL;
    }

    write_data(st, "ARP\n");
    write_data(st, "\tHardware Type: ");

    switch( htons(pkt->hrd) ) {
        case 1:
            write_data(st, "Ethernet (10Mb)\n");
            break;
        case 6:
            write_data(st, "IEEE 802 Networds\n");
            break;
        case 7:
            write_data(st, "ARCNET\n");
            break;
        case 15:
            write_data(st, "Frame Relay\n");
            break;
        case 16:
            write_data(st, "ATM\n");
            break;
        case 17:
            write_data(st, "HDLC\n");
            break;
        case 18:
            write_data(st, "Fibre Channel\n");
            break;
        case 19:
            write_data(st, "ATM\n");
            break;
        case 20:
            write_data(st, "Serial Line\n");
            break;
        default:
            write_data(st, "Unknown Hardware type\n");
            break;
    }

    if ( pkt->prot == 0x0008 ) {
        write_data(st, "\tProtocol Type: IPv4\n");
    } else {
        write_data(st, "\tUnknown protocol type\n");
    }

    write_data(st, "\tHardware Address Length: %d\n", pkt->hln);
    write_data(st, "\tProtocol Address Length: %d\n", pkt->pln);

    write_data(st, "\tOpcode: ");

    switch ( htons(pkt->opcode) ) {
        case 1:
            write_data(st, "ARP Request\n");
            break;
        case 2:
            write_data(st, "ARP Reply\n");
            break;
        case 3:
            write_data(st, "RARP Request\n");
            break;
        case 4:
            write_data(st, "RARP Reply\n");
            break;
        case 5:
            write_data(st, "DRARP Request\n");
            break;
        case 6:
            write_data(st, "DRARP Reply\n");
            break;
        case 7:
            write_data(st, "DRARP Error\n");
            break;
        case 8:
            write_data(st, "InARP Request\n");
            break;
        case 9:
            write_data(st, "InArp Reply\n");
            break;
        default:
            write_data(st, "Unknown\n");
            break;
    }

    if ( htons(pkt->hrd) == 1 ) {
        if ( htons(pkt->prot) == 0x0800 ) {
            write_data(st, "\tSender MAC: ");

            for ( int i = 0; i < pkt->hln; i++) {
                write_data(st, "%.2x:", pkt->sender_ha[i]);
            }

            

            write_data(st, "\n\tSender IP: ");
            write_data(st, "%d.%d.%d.%d\n", pkt->sender_pa[0], pkt->sender_pa[1], pkt->sender_pa[2], pkt->sender_pa[3]);

            write_data(st, "\tDest MAC: ");

            for ( int i = 0; i < pkt->hln; i++) {
                write_data(st, "%.2x:", pkt->dest_ha[i]);
            }

            

            write_data(st, "\n\tDest IP: ");
            write_data(st, "%d.%d.%d.%d\n", pkt->dest_pa[0], pkt->dest_pa[1], pkt->dest_pa[2], pkt->dest_pa[3]);
        }
    } else {
        write_data(st, "\tAddress type not handled\n");
    }

    return pkt;
}