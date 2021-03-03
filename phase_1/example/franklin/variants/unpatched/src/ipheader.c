#include "ipheader.h"
#include <malloc.h>
#include "igmp.h"

extern stream *st;

int parse_eigrp( )
{
    uint8_t version;
    uint8_t opcode;
    uint16_t checksum;
    uint32_t flags;
    uint32_t sequence;
    uint32_t acknowledge;
    uint16_t vr_id;
    uint16_t auto_sys;
    uint16_t parameters;
    uint16_t length;
    uint8_t *k;

    uint16_t software_version;
    uint16_t sv_length;
    uint16_t release;
    uint16_t tlv_version;

    if ( !st ) {
        return 0;
    }

    if ( !get_bits( st, &version, 8 ) ) {
        write_data(st, "\tInsufficient data: 1\n");
        return 0;
    }

    if ( !get_bits( st, &opcode, 8 ) ) {
        write_data(st, "\tInsufficient data: 2\n");
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&checksum, 16 ) ) {
        write_data(st, "\tInsufficient data: 3\n");
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&flags, 32 ) ) {
        write_data(st, "\tInsufficient data: 4\n");
        return 0;
    }
    if ( !get_bits( st, (uint8_t*)&sequence, 32 ) ) {
        write_data(st, "\tInsufficient data: 5\n");
        return 0;
    }
    if ( !get_bits( st, (uint8_t*)&acknowledge, 32 ) ) {
        write_data(st, "\tInsufficient data: 5\n");
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&vr_id, 16 ) ) {
        write_data(st, "\tInsufficient data: 7\n");
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&auto_sys, 16 ) ) {
        write_data(st, "\tInsufficient data: 8\n");
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&parameters, 16 ) ) {
        write_data(st, "\tInsufficient data: 9\n");
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&length, 16 ) ) {
        write_data(st, "\tInsufficient data: 11\n");
        return 0;
    }

    length = htons( length );

    
    k = malloc( length );

    if ( !k ) {
        return 0;
    }

    memset( k, 0, length);

    
    length -= 4;

    if ( !get_bits( st, k, length * 8 ) ) {
        write_data(st, "\tInsufficient data: 12\n");
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&software_version, 16 ) ) {
        write_data(st, "\tInsufficient data: 13\n");
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&sv_length, 16 ) ) {
        write_data(st, "\tInsufficient data: 14\n");
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&release, 16 ) ) {
        write_data(st, "\tInsufficient data: 15\n");
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&tlv_version, 16 ) ) {
        write_data(st, "\tInsufficient data: 16\n");
        return 0;
    }


    write_data(st, "EIGRP:\n");
    write_data(st, "\tVersion: %d\n", version);
    write_data(st, "\tOpcode: %d\n", opcode);
    write_data(st, "\tChecksum: %x\n", checksum);
    write_data(st, "\tFlags: %.8x\n", flags);
    write_data(st, "\tSequence: %.8x\n", sequence);
    write_data(st, "\tAcknowledge: %.8x\n", acknowledge);
    write_data(st, "\tVirtual Router ID: %.4x\n", vr_id);

    write_data(st, "\tAutonomous System: %d\n", htons(auto_sys));
    write_data(st, "\tParameters: %.4x\n", parameters);
    write_data(st, "\tLength: %.4x\n", length);
    write_data(st, "\tParams: ");

    for( int i = 0; i < length; i++) {
        write_data(st, "%.2x ", k[i]);
    }

    write_data(st, "\n");

    write_data(st, "\tSoftware Version: %x\n", htons(software_version) );
    write_data(st, "\tLength: %x\n", htons(sv_length) );
    write_data(st, "\tRelease: %d.%d\n", (release&0xff), release >> 8);
    write_data(st, "\tTLV Version: %d.%d\n", (tlv_version&0xff), tlv_version>>8);

    return 1;
}

void print_ipv6_addr( char *addr )
{
    uint16_t t;

    if ( !addr ) {
        return;
    }

    memcpy( &t, addr, 2 );

    write_data(st, "%.4x:", htons(t));

    memcpy( &t, addr+2, 2 );

    write_data(st, "%.4x:", htons(t));

    memcpy( &t, addr+4, 2 );

    write_data(st, "%.4x:", htons(t));

    memcpy( &t, addr+6, 2 );

    write_data(st, "%.4x:", htons(t));

    memcpy( &t, addr+8, 2 );

    write_data(st, "%.4x:", htons(t));

    memcpy( &t, addr+10, 2 );

    write_data(st, "%.4x:", htons(t));

    memcpy( &t, addr+12, 2 );

    write_data(st, "%.4x:", htons(t));

    memcpy( &t, addr+14, 2 );

    write_data(st, "%.4x\n", htons(t));

    return;
}

void print_ipv4_addr ( uint32_t addr )
{
    write_data(st, "%d.%d.%d.%d", addr & 0xff, ( addr >> 8 ) & 0xff, (addr >> 16) & 0xff, (addr >> 24));
}

iphv4 * parse_ipv4_header( )
{
    iphv4 * header = NULL;
    int ihl = 0;
    uint8_t version;

    if ( !st ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&version, 4 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&ihl, 4 ) ) {
        return NULL;
    }

    return_bits( st, 8 );

    if ( version != 4 ) {
        write_data(st, "[ERROR] Incorrect version: %d\n", version);
        return NULL;
    }

    if ( ihl != 5 ) {
        write_data(st, "[ERROR] I don't parse options yet\n");
        return NULL;
    }

    header = malloc( ihl * 8 );

    if ( !header ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->version, 4 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->ihl, 4 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->dscp, 6 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->ecn, 2 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->length, 16 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->id, 16 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->flags, 3 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->offset, 13 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->ttl, 8 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->protocol, 8 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->checksum, 16 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->source, 32 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&header->dest, 32 ) ) {
        return NULL;
    }

    write_data(st, "IPv4 Header\n");
    write_data(st, "\tVersion: %d\n", header->version);
    write_data(st, "\tLength: %d\n", header->ihl * 4);
    write_data(st, "\tDSCP: %x\n", header->dscp);
    write_data(st, "\tECN: %x\n", header->ecn);
    write_data(st, "\tTotal Length: %d\n", htons(header->length));
    write_data(st, "\tID: %x\n", header->id);
    write_data(st, "\tFlags: %x\n", header->flags);
    write_data(st, "\tFragment offset: %x\n", header->offset);
    write_data(st, "\tTTL: %d\n", header->ttl);
    write_data(st, "\tProtocol: %d\n", header->protocol); 
    write_data(st, "\tChecksum: %x\n", header->checksum);
    write_data(st, "\tSource: ");
    print_ipv4_addr( header->source );
    write_data(st, "\n");

    write_data(st, "\tDest: " );
    print_ipv4_addr( header->dest );
    write_data(st, "\n");

    switch ( header->protocol ) {
        case 1:
            if ( !parse_icmpv4( st ) ) {
                return NULL;
            }
            break;
        case 2:
            if ( !parse_igmp( st ) ) {
                return NULL;
            }
            break;
        case 6:
            header->ptcp = parse_tcp_header( st );

            if ( !header->ptcp ) {
                return NULL;
            }
            break;
        case 17:
            header->pudp = parse_udp_header( st );
            if ( !header->pudp) {
                return NULL;
            }
            break;
        case 41:
            header->ipv6 = parse_ipv6_header( st );
            if ( !header->ipv6) {
                return NULL;
            }
            break;
        default:
            write_data(st, "Not Parsed\n");
            return NULL;
            break;
    }

    return header;
}

int handle_ipv6_extension( uint32_t ext )
{
    uint8_t next_header;
    uint8_t byte;
    uint8_t *data;

    if ( !st ) {
        return 0;
    }

    switch ( ext ) {
        case 0:
            write_data(st, "\tHop-by-Hop Option\n");

            if ( !get_bits( st, &next_header, 8) ) {
                return 0;
            }

            write_data(st, "\tNext Header: %x\n", next_header);

            if ( !get_bits( st, &byte, 8) ) {
                return 0;
            }

            
            if ( byte == 0 ) {
                byte = 8;
            }

            
            byte -= 2;

            if ( bits_remaining( st ) < byte * 8 ) {
                return 0;
            }

            data = malloc( byte );

            if ( !data ) {
                return 0;
            }

            
            get_bits( st, data, byte * 8);

            write_data(st, "\tData: ");
            for ( int i = 0; i < byte; i++) {
                write_data(st, "%.2x ", data[i]);
            }

            write_data(st, "\n");

            free( data );
            break;
        default:
            return ext;
            break;
    }

    return handle_ipv6_extension( next_header );
}

iphv6 *parse_ipv6_header( )
{
    iphv6 *ip6h = NULL;
    tcp *ptcp;

    if ( st == NULL ) {
        return NULL;
    }

    ip6h = malloc( sizeof(iphv6) );

    if ( !ip6h ) {
        write_data(st, "Failed to allocate ipv6 struct\n");
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&ip6h->version, 4 ) ) {
        return NULL;
    }

    if ( ip6h->version != 6 ) {
        write_data(st, "[ERROR] Invalid version value for IPv6 packet: %d\n", ip6h->version );
        free( ip6h );
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&ip6h->traffic_class, 8 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&ip6h->flow_label, 20 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&ip6h->payload_length, 16 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&ip6h->next_header, 8 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&ip6h->hop_limit, 8 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&ip6h->source, 128 ) ) {
        return NULL;
    }

    if ( !get_bits( st, (uint8_t*)&ip6h->dest, 128 ) ) {
        return NULL;
    }

    write_data(st, "IPv6\n");
    write_data(st, "\tVersion: %d\n", ip6h->version);
    write_data(st, "\tTraffic Class: %x\n", ip6h->traffic_class);
    write_data(st, "\tFlow Label: %x\n", ip6h->flow_label);
    write_data(st, "\tPayload length: %d\n", htons(ip6h->payload_length));
    write_data(st, "\tNext Header: " );

    switch ( ip6h->next_header ) {
        case 0:
        case 60: 
        case 43:
        case 44:
        case 51:
        case 50:
        case 135:
        case 139:
        case 150:
        case 253:
        case 254:
            ip6h->next_header = handle_ipv6_extension( ip6h->next_header);
            break;
        case 58:
            write_data(st, "ICMPv6\n");
            break;
        case 6:
            write_data(st, "TCP\n");
            break;
        case 88:
            write_data(st, "EIGRP\n");
            break;
        default: 
            write_data(st, "Unknown -- %d\n", ip6h->next_header);
            break;
    }

    write_data(st, "\tHop Limit: %d\n", ip6h->hop_limit);
    write_data(st, "\tSource: ");

    print_ipv6_addr( (char*)ip6h->source);

    write_data(st, "\tDest: ");

    print_ipv6_addr( (char*)ip6h->dest);

    switch ( ip6h->next_header ) {
        case 0:
        case 60: 
        case 43:
        case 44:
        case 51:
        case 50:
        case 135:
        case 139:
        case 150:
        case 253:
        case 254:
            write_data(st, "IPv6 Extension [Not Handled]\n");
            
            return NULL;
            break;
        case 58:
            
            if ( !parse_icmpv6( htons(ip6h->payload_length) ) ) {
                write_data(st, "[ERROR] Failed to parse icmpv6 header\n");
                return NULL;
            }
            break;
        case 6:
            ptcp = parse_tcp_header( st );
            if ( !ptcp ) {
                write_data(st, "[ERROR] Failed to parse tcp header\n");
            }
            break;
        case 88:
            if ( !parse_eigrp( ) ) {
                write_data( st, "[ERROR] Failed to parse eigrp\n");
            }
            break;
        default: 
            write_data(st, "Unknown -- %d\n", ip6h->next_header);
            break;
    }

    
    return ip6h;
}