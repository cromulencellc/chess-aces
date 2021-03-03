#include "icmp.h"
#include "ipheader.h"
#include <alloca.h>

extern stream *st;

int parse_icmpv6_options( )
{
    uint8_t code;
    uint8_t byte;
    uint16_t two_bytes;
    uint64_t length;
    uint64_t four_bytes;
    uint8_t *data;
    uint8_t addr[240];
    uint8_t *junk;
    uint64_t i;

    if ( !st ) {
        return 0;
    }

    if ( bits_remaining( st ) < 8 ) {
        return 0;
    }

    get_bits( st, &code, 8);

    switch (code ) {
        case 1:
            write_data(st, "\tICMPv6 Option: Source Link Layer Address\n");

            if ( !get_bits( st, &byte, 8) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            length = byte * 8;

            write_data(st, "\tLength: %d\n", length);

            
            if ( bits_remaining( st ) < (length - 2) * 8 ) {
                write_data(st, "\tInsufficient length remaining\n");
                return 0;
            }

            data = malloc( length );

            if ( !data ) {
                return 0;
            }

            get_bits( st, data, (length-2) * 8);

            write_data(st, "\tAddress: ");

            for ( int i = 0; i < length-2; i++) {
                write_data(st, "%.2x ", data[i]);
            }

            write_data(st, "\n");

            free( data );
            break;
        case 2:
            write_data(st, "\tICMPv6 Option: Target link-layer Address\n");

            if ( !get_bits( st, &byte, 8) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            length = byte * 8;

            write_data(st, "\tLength: %d\n", length);

            length -= 2;

            data = malloc( length );

            if ( !data ) {
                return 0;
            }

            if ( !get_bits( st, data, length * 8 ) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                free(data);
                return 0;
            }

            write_data(st, "\tLink Layer Addr: ");

            for ( int i =0 ; i < length; i++) {
                write_data(st, "%.2x ", data[i]);
            }

            write_data(st, "\n");

            free(data);
            break;
        case 3:
            write_data(st, "\tICMPv6 Option: Prefix Information\n");

            if ( !get_bits( st, &byte, 8) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            length = byte * 8;

            write_data(st, "\tLength: %d\n", length);

            uint8_t prefix_length = 0;

            if ( !get_bits( st, &prefix_length, 8) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tPrefix Length: %d\n", prefix_length );

            
            length -= 16;

            
            if ( length > prefix_length / 8 ) {
                prefix_length = length * 8;
            }

            if ( !get_bits( st, &byte, 8) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tFlags: %x\n", byte);

            if ( !get_bits( st, (uint8_t*)&four_bytes, 32) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tValid Lifetime: %d\n", htonl(four_bytes));

            if ( !get_bits( st, (uint8_t*)&four_bytes, 32) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tPreferred Lifetime: %d\n", htonl(four_bytes));

            
            if ( !get_bits( st, (uint8_t*)&four_bytes, 32) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            data = malloc( (prefix_length + 8 )/ 8 );

            if ( !data ) {
                return 0;
            }

            if ( !get_bits( st, data, prefix_length) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                free( data );
                return 0;
            }

            if ( prefix_length == 128 ) {
                write_data(st, "\tPrefix: ");
                print_ipv6_addr( (char*)data);
            } else {
                for ( i = 0; i < prefix_length / 8; i++) {
                    write_data(st, "%.2x ", data[i]);
                }

                write_data(st, "\n");
            }

            free( data);
            break;
        case 5:
            write_data(st, "\tICMPv6 Option: MTU\n");

            if ( !get_bits( st, &byte, 8) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            length = byte * 8;

            write_data(st, "\tLength: %d\n", length);

            
            if ( !get_bits( st, (uint8_t*)&two_bytes, 16) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            if ( !get_bits( st, (uint8_t*)&four_bytes, 32) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tMTU: %d\n", htonl(four_bytes));
            break;
        case 14:
            write_data(st, "\tICMPv6 Option: Nonce\n");

            if ( !get_bits( st, &byte, 8) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            length = byte * 8;

            write_data(st, "\tLength: %d\n", length);

            
            if ( !get_bits( st, (uint8_t*)&two_bytes, 16) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            if ( !get_bits( st, (uint8_t*)&four_bytes, 32) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tNonce: %.8x\n", htonl(four_bytes));
            break;
        case 24:
            write_data(st, "\tICMPv6 Option: Route Information\n");

            if ( !get_bits( st, &byte, 8) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            length = byte * 8;

            if ( bits_remaining( st ) < ((length * 8) - 0x10)) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tLength: %d\n", length);

            prefix_length = 0;

            if ( !get_bits( st, &prefix_length, 8) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tPrefix Length: %d\n", prefix_length);

            if ( !get_bits( st, &byte, 8) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tFlags: %x\n", byte);

            if ( !get_bits( st, (uint8_t*)&four_bytes, 32) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tRoute Lifetime: %d\n", htonl(four_bytes));

            memset ( addr, 0, 240);

            if ( !get_bits( st, (uint8_t*)&addr, 128) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tPrefix: ");
            print_ipv6_addr( (char*)&addr);
            break;
        case 25:
            write_data(st, "\tICMPv6 Option: Recursive DNS Server\n");

            if ( !get_bits( st, &byte, 8) ) {
                return 0;
            }

            length = byte * 8;

            if ( bits_remaining( st ) < ((length * 8) - 0x10)) {
                return 0;
            }

            write_data(st, "\tLength: %d\n", length);

            
            if ( !get_bits( st, (uint8_t*)&two_bytes, 16) ) {
                return 0;
            }

            if ( !get_bits( st, (uint8_t*)&four_bytes, 32) ) {
                return 0;
            }

            write_data(st, "\tLifetime: %d\n", htonl(four_bytes));

            length -= 8;

            memset(addr, 0, 240 );

            while ( length > 0) {
                if ( !get_bits( st, (uint8_t*)&addr, 128) ) {
                    return 0;
                }

                write_data(st, "\tRecursive DNS Server: ");
                print_ipv6_addr( (char*)&addr);

                length -= 16;
            }
            break;
        case 31:
            write_data(st, "\tICMPv6 Option: DNS Search List Option\n");

            if ( !get_bits( st, &byte, 8) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            length = byte * 8;

            if ( bits_remaining( st ) < ((length * 8) - 0x10)) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tLength: %d\n", length);

            
            if ( !get_bits( st, (uint8_t*)&two_bytes, 16) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            if ( !get_bits( st, (uint8_t*)&four_bytes, 32) ) {
                write_data(st, "\t[ERROR] Insufficient data in options\n");
                return 0;
            }

            write_data(st, "\tLifetime: %d\n", htonl(four_bytes) );

            
            junk = alloca(length);
            memset( junk, 0, length );

            uint64_t j = 0;

            while ( get_bits( st, &byte, 8) ) {

                if ( !byte ) {
                    return 0;
                }

                j = 0;
                i = 0;

                while ( byte && !i) {

                    if ( ! get_bits( st, junk + j, byte * 8) ) {
                        i = 1;
                        continue;
                    }

                    j += byte;

                    
                    if ( j*8 > bits_remaining(st)) {
                        i = 1;
                        continue;
                    }

                    if ( !get_bits( st, &byte, 8) ) {
                        i = 1;
                        continue;
                    }
                }

                if ( i ) {
                    write_data(st, "\t[ERROR] Insufficient data in options\n");
                    return 0;
                }

                write_data(st, "\tDomain Name: %s\n", junk);
            }
            break;
        default:
            write_data(st, "\tOption not yet parsed: %d\n", code);
            return 0;
            break;
    }

    return 1;
}

int parse_icmpv6( uint16_t length)
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint8_t byte;
    uint16_t two_bytes;
    uint32_t four_bytes;
    uint8_t *addr = alloca(16);
    int i,j;

    if ( st == NULL ) {
        return 0;
    }

    if ( !get_bits( st, &type, 8) ) {
        return 0;
    }

    if ( !get_bits( st, &code, 8) ) {
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&checksum, 16) ) {
        return 0;
    }

    write_data(st, "ICMPv6: \n");
    switch ( type ) {
        case 1:
            write_data(st, "\tDestination Unreachable\n");
            break;
        case 2:
            write_data(st, "\tPacket Too Big\n");
            break;
        case 3:
            write_data(st, "\tTime Exceeded\n");
            break;
        case 4:
            write_data(st, "\tParameter Problem\n");
            break;
        case 100:
        case 101:
            write_data(st, "\tPrivate Experimentation\n");
            break;
        case 127:
            write_data(st, "\tReserved\n");
            break;
        case 128:
            write_data(st, "\tEcho Request\n");
            break;
        case 129:
            write_data(st, "\tEcho Reply\n");
            break;
        case 130:
            write_data(st, "\tMulticast Listener Query\n");
            break;
        case 131:
            write_data(st, "\tMulticast Listener Report\n");
            break;
        case 132:
            write_data(st, "\tMulticast Listener Done\n");
            break;
        case 133:
            write_data(st, "\tRouter Solicitation\n");
            break;
        case 134:
            write_data(st, "\tRouter Advertisement\n");
            break;
        case 135:
            write_data(st, "\tNeighbor Solicitation\n");
            break;
        case 136:
            write_data(st, "\tNeighbor Advertisement\n");
            break;
        case 137:
            write_data(st, "\tRedirect Message\n");
            break;
        case 138:
            write_data(st, "\tRouter Renumbering\n");
            break;
        case 139:
            write_data(st, "\tICMP Node Information Query\n");
            break;
        case 140:
            write_data(st, "\tICMP Node Information Reponse\n");
            break;
        case 141:
            write_data(st, "\tInverse Neighbor Discovery Solicitation Message\n");
            break;
        case 142:
            write_data(st, "\tInverse Neighbor Discovery Advertisemenet Message\n");
            break;
        case 143:
            write_data(st, "\tMulticast Listener Discovery reports\n");
            break;
        case 144:
            write_data(st, "\tHome Agent Address Discovery Request\n");
            break;
        case 145:
            write_data(st, "\tHome Agent Address Discovery Reply Message\n");
            break;
        case 146:
            write_data(st, "\tMobile Prefix Solicitation\n");
            break;
        case 147:
            write_data(st, "\tMobile Prefix Advertisement\n");
            break;
        case 148:
            write_data(st, "\tCertification Path Solicitation\n");
            break;
        case 149:
            write_data(st, "\tCertification Path Advertisement\n");
            break;
        case 151:
            write_data(st, "\tMulticast Router Advertisement\n");
            break;
        case 152:
            write_data(st, "\tMulticast Router Solicitation\n");
            break;
        case 153:
            write_data(st, "\tMulticast Router Termination\n");
            break;
        case 155:
            write_data(st, "\tRPL Control Message\n");
            break;
        case 200:
        case 201:
            write_data(st, "\tPrivate Experimentation\n");
            break;
        case 255:
            write_data(st, "\tReserved for Expansion\n");
            break;
        default:
            write_data(st, "\tUnknown\n");
            break;
    };

    write_data(st, "\tChecksum: 0x%.4x\n", htons(checksum));

    switch ( type ) {
        case 1:
            write_data(st, "\tDestination Unreachable\n");
            break;
        case 2:
            write_data(st, "\tCode: %d\n", code);
            write_data(st, "\tChecksum: %.4x\n", checksum);

            if ( !get_bits( st, (uint8_t*)&four_bytes, 32) ) {
                return 0;
            }

            write_data(st, "\tMTU: %d\n", htonl(four_bytes));

            break;
        case 3:
            write_data(st, "\tTime Exceeded\n");
            break;
        case 4:
            write_data(st, "\tParameter Problem\n");
            break;
        case 100:
        case 101:
            write_data(st, "\tPrivate Experimentation\n");
            break;
        case 127:
            write_data(st, "\tReserved\n");
            break;
        case 128:
            if ( !get_bits( st, (uint8_t*)&two_bytes, 16 ) ) {
                return 0;
            }

            write_data(st, "\tIdentifier: %.4x\n", two_bytes);

            if ( !get_bits( st, (uint8_t*)&two_bytes, 16 ) ) {
                return 0;
            }

            write_data(st, "\tSequence: %.4x\n", two_bytes);

            write_data(st, "\tData: ");

            while ( bits_remaining( st ) >= 8 ) {
                get_bits( st, &byte, 8);

                write_data(st, "%.2x ", byte);
            }

            write_data(st, "\n");

            while ( parse_icmpv6_options( ) ) { }
            break;
        case 129:
            if ( !get_bits( st, (uint8_t*)&two_bytes, 16 ) ) {
                return 0;
            }

            write_data(st, "\tIdentifier: %.4x\n", two_bytes);

            if ( !get_bits( st, (uint8_t*)&two_bytes, 16 ) ) {
                return 0;
            }

            write_data(st, "\tSequence: %.4x\n", two_bytes);

            write_data(st, "\tData: ");

            while ( bits_remaining( st ) >= 8 ) {
                get_bits( st, &byte, 8);

                write_data(st, "%.2x ", byte);
            }

            write_data(st, "\n");

            while ( parse_icmpv6_options( ) ) { }
            break;
        case 130:
            write_data(st, "\tMulticast Listener Query\n");
            break;
        case 131:
            write_data(st, "\tMulticast Listener Report\n");
            break;
        case 132:
            write_data(st, "\tMulticast Listener Done\n");
            break;
        case 133:
            write_data(st, "\tRouter Solicitation\n");
            break;
        case 134:
            write_data(st, "\tRouter Advertisement\n");

            if ( !get_bits( st, &byte, 8 ) ) {
                return 0;
            }

            write_data(st, "\tCurrent Hop Limit: %d\n", byte);

            if ( !get_bits( st, &byte, 8 ) ) {
                return 0;
            }

            write_data(st, "\tFlags: %.2x\n", byte);

            if ( !get_bits( st, (uint8_t*)&two_bytes, 16 ) ) {
                return 0;
            }

            write_data(st, "\tRouter Lifetime: %d\n", htons(two_bytes));

            if ( !get_bits( st, (uint8_t*)&four_bytes, 32 ) ) {
                return 0;
            }

            write_data(st, "\tReachable time: %d\n", htonl(four_bytes));

            if ( !get_bits( st, (uint8_t*)&four_bytes, 32 ) ) {
                return 0;
            }

            write_data(st, "\tRetrans timer: %.8x\n", htonl(four_bytes));

            while ( parse_icmpv6_options( ) ) { }
            break;
        case 135:
            write_data(st, "\tCode: %d\n", code);
            
            if ( !get_bits( st, (uint8_t*)&four_bytes, 32 ) ) {
                return 0;
            }

            if ( !get_bits( st, addr, 128 ) ) {
                return 0;
            }

            write_data(st, "\tAddress: ");
            print_ipv6_addr( (char*)addr);

            while ( parse_icmpv6_options( ) ) { }

            break;
        case 136:
            write_data(st, "\tCode: %d\n", code);

            if ( !get_bits( st, (uint8_t*)&four_bytes, 32 ) ) {
                write_data(st, "\tInsufficient Data: 3\n");
                return 0;
            }

            
            write_data(st, "\tFlags: %.8x\n", four_bytes);


            if ( !get_bits( st, addr, 128 ) ) {
                write_data(st, "\tInsufficient Data: 4\n");
                return 0;
            }

            write_data(st, "\tTarget Addr: ");

            print_ipv6_addr( (char*)addr);

            while ( parse_icmpv6_options( ) ) { }

            break;
        case 137:
            write_data(st, "\tRedirect Message\n");
            break;
        case 138:
            write_data(st, "\tRouter Renumbering\n");
            break;
        case 139:
            write_data(st, "\tICMP Node Information Query\n");

            write_data(st, "\tCode: ");
            switch( code ) {
                case 0:
                    write_data(st, "Query IPv6 Addresses\n");

                    uint16_t qtype;

                    if (!get_bits( st, (uint8_t*)&qtype, 16) ) {
                        return 0;
                    }

                    qtype = htons(qtype);

                    write_data(st, "\tQtype: ");

                    switch ( qtype ) {
                        case 0:
                            write_data(st, "NOOP\n");
                            break;
                        case 1:
                            write_data(st, "Unused\n");
                            break;
                        case 2:
                            write_data(st, "Node Name\n");
                            break;
                        case 3:
                            write_data(st, "Node Addresses\n");
                            break;
                        case 4:
                            write_data(st, "IPv4 Addresses\n");
                            break;
                        default:
                            write_data(st, "Unknown\n");
                            break;
                    }

                    uint16_t flags;

                    if (!get_bits( st, (uint8_t*)&flags, 16) ) {
                        return 0;
                    }

                    write_data(st, "\tFlags: %.4x\n", htons(flags));

                    uint32_t nonce;

                    if (!get_bits( st, (uint8_t*)&nonce, 32) ) {
                        return 0;
                    }

                    write_data(st, "\tFlags: %.8x\n", htonl(nonce));

                    if (!get_bits( st, addr, 128) ) {
                        return 0;
                    }

                    write_data(st, "\tIPv6 subject address: ");

                    print_ipv6_addr( (char*)addr );

                    break;
                default:
                    write_data(st, "%d\n", code);
                    break;
            }

            break;
        case 140:
            write_data(st, "\tCode: ");

            switch (code ) {
                case 0:
                    write_data(st, "Successful\n");
                    break;
                case 1:
                    write_data(st, "Refuse to answer\n");
                    break;
                case 2:
                    write_data(st, "Qtype query type unknown\n");
                    break;
                default:
                    write_data(st, "%d\n", code);
                    break;
            }

            uint16_t qtype;

            if (!get_bits( st, (uint8_t*)&qtype, 16) ) {
                return 0;
            }

            qtype = htons(qtype);

            uint16_t flags;

            if (!get_bits( st, (uint8_t*)&flags, 16) ) {
                return 0;
            }

            uint64_t nonce;

            if (!get_bits( st, (uint8_t*)&nonce, 64) ) {
                return 0;
            }

            write_data(st, "\tQtype: ");

            switch ( qtype ) {
                case 0:
                    write_data(st, "NOOP\n");
                    break;
                case 1:
                    write_data(st, "Unused\n");
                    break;
                case 2:
                    write_data(st, "Node Name\n");

                    uint32_t ttl;

                    if (!get_bits( st, (uint8_t*)&ttl, 32) ) {
                        return 0;
                    }

                    write_data(st, "\tTTL: %.8x\n", ttl);

                    uint8_t l;

                    if ( !get_bits( st, &l, 8) ) {
                        return 0;
                    }

                    uint8_t *data;

                    while ( l ) {
                        data = malloc( l + 1 );

                        if ( !data ) {
                            return 0;
                        }

                        if ( !get_bits( st, data, (l+1) * 8 ) ) {
                            free(data);
                            return 0;
                        }

                        write_data(st, "\tNode Name: %s\n", data);
                        free(data);

                        if ( !get_bits( st, &l, 8) ) {
                            return 0;
                        }
                    }
                    break;
                case 3:
                    write_data(st, "Node Addresses\n");
                    break;
                case 4:
                    write_data(st, "IPv4 Addresses\n");

                    write_data(st, "\tFlags: %.4x\n", htons(flags));

                    write_data(st, "\tNonce: %.16lx\n", nonce);
                    break;
                default:
                    write_data(st, "Unknown\n");
                    break;
            }

            

            uint32_t br = bits_remaining( st ) / 8;

            if ( !br ) {
                return 1;
            }

            write_data(st, "\tData: ");

            for ( int i = 0; i < br; i ++ ) {
                get_bits( st, &byte, 8);

                write_data(st, "%.2x ", byte);
            }

            write_data(st, "\n");

            break;
        case 141:
            write_data(st, "\tInverse Neighbor Discovery Solicitation Message\n");
            break;
        case 142:
            write_data(st, "\tInverse Neighbor Discovery Advertisemenet Message\n");
            break;
        case 143:
            
            if ( !get_bits( st, (uint8_t*)&two_bytes, 16) ) {
                write_data(st, "\tInsufficient data\n");
                return 0;
            }

            if ( !get_bits( st, (uint8_t*)&two_bytes, 16) ) {
                write_data(st, "\tInsufficient data\n");
                return 0;
            }

            two_bytes = htons( two_bytes );

            write_data(st, "\tNumber of records: %d\n", two_bytes);

            
            for( i = 0, j=two_bytes; i < j; i++ ) {
                if ( !get_bits( st, (uint8_t*)&byte, 8) ) {
                    write_data(st, "\tInsufficient data\n");
                    return 0;
                }

                write_data(st, "\tRecord Type: %d\n", byte);

                if ( !get_bits( st, (uint8_t*)&byte, 8) ) {
                    write_data(st, "\tInsufficient data\n");
                    return 0;
                }

                write_data(st, "\tAux data len: %d\n", byte);

                if ( !get_bits( st, (uint8_t*)&two_bytes, 16) ) {
                    write_data(st, "\tInsufficient data\n");
                    return 0;
                }

                write_data(st, "\tNumber of sources: %d\n", htons(two_bytes));

                if ( !get_bits( st, addr, 128) ) {
                    write_data(st, "\tInsufficient data\n");
                    return 0;
                }

                write_data(st, "\tMulticast Address: ");
                print_ipv6_addr( (char*)addr );
            }

            while ( parse_icmpv6_options( ) ) { }

            break;
        case 144:
            write_data(st, "\tHome Agent Address Discovery Request\n");
            break;
        case 145:
            write_data(st, "\tHome Agent Address Discovery Reply Message\n");
            break;
        case 146:
            write_data(st, "\tMobile Prefix Solicitation\n");
            break;
        case 147:
            write_data(st, "\tMobile Prefix Advertisement\n");
            break;
        case 148:
            write_data(st, "\tCertification Path Solicitation\n");
            break;
        case 149:
            write_data(st, "\tCertification Path Advertisement\n");
            break;
        case 151:
            write_data(st, "\tMulticase Router Advertisement\n");
            break;
        case 152:
            write_data(st, "\tMulticast Router Solicitation\n");
            break;
        case 153:
            write_data(st, "\tMulticast Router Termination\n");
            break;
        case 155:
            write_data(st, "\tRPL Control Message\n");
            break;
        case 200:
        case 201:
            write_data(st, "\tPrivate Experimentation\n");
            break;
        case 255:
            write_data(st, "\tReserved for Expansion\n");
            break;
        default:
            write_data(st, "\tUnknown\n");
            break;
    };

    return 1;
}

int parse_icmpv4( )
{
    icmp *picmp = NULL;

    if ( st == NULL ) {
        return 0;
    }

    picmp = malloc( sizeof(icmp) );

    if ( picmp == NULL ) {
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&picmp->type, 8) ) {
        free( picmp );
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&picmp->code, 8) ) {
        free( picmp );
        return 0;
    }

    if ( !get_bits( st, (uint8_t*)&picmp->checksum, 16) ) {
        free( picmp );
        return 0;
    }

    write_data(st, "ICMP\n");

    write_data(st, "\tType: ");

    switch (picmp->type) {
        case 0:
            write_data(st, "\tEcho Reply\n");
            break;
        case 1:
        case 2:
            write_data(st, "\tReserved\n");
            break;
        case 3:
            write_data(st, "\tDestination Unreachable\n");
            write_data(st, "\tCode: ");
            switch (picmp->code ) {
                case 0:
                    write_data(st, "\tDestination network unreachable\n");
                    break;
                case 1:
                    write_data(st, "\tDestination host unreachable\n");
                    break;
                case 2:
                    write_data(st, "\tDestination protocol unreachable\n");
                    break;
                case 3:
                    write_data(st, "\tDestination port unreachable\n");
                    break;
                case 4:
                    write_data(st, "\tFragmentation required, and DF flag set\n");
                    break;
                case 5:
                    write_data(st, "\tSource route failed\n");
                    break;
                case 6:
                    write_data(st, "\tDestination network unknown\n");
                    break;
                case 7:
                    write_data(st, "\tDestination host unknown\n");
                    break;
                case 8:
                    write_data(st, "\tSource host isolated\n");
                    break;
                case 9:
                    write_data(st, "\tNetwork administratively prohibited\n");
                    break;
                case 10:
                    write_data(st, "\tHost administratively prohibited\n");
                    break;
                case 11:
                    write_data(st, "\tNetwork unreachable for ToS\n");
                    break;
                case 12:
                    write_data(st, "\tHost unreachable for ToS\n");
                    break;
                case 13:
                    write_data(st, "\tCommunication administratively prohibited\n");
                    break;
                case 14:
                    write_data(st, "\tHost Precedence Violation\n");
                    break;
                case 15:
                    write_data(st, "\tPrecedence cutoff in effect\n");
                    break;
                default:
                    write_data(st, "\tUnknown\n");
                    break;
            }
            break;
        case 4:
            write_data(st, "\tSource Quench\n");
            break;
        case 5:
            write_data(st, "\tRedirect Message\n");
            break;
        case 6:
            write_data(st, "\tDeprecated\n");
            break;
        case 7:
            write_data(st, "\tReserved\n");
            break;
        case 8:
            write_data(st, "\tEcho Request\n");
            break;
        case 9:
            write_data(st, "\tRouter Advertisement");
            break;
        case 10:
            write_data(st, "\tRouter Solicitation\n");
            break;
        case 11:
            write_data(st, "\tTime Exceeded\n");
            break;
        case 12:
            write_data(st, "\tParameter Problem: Bad IP header\n");
            break;
        case 13:
            write_data(st, "\tTimestamp\n");
            break;
        case 14:
            write_data(st, "\tTimestamp Reply\n");
            break;
        case 15:
            write_data(st, "\tInformation Request\n");
            break;
        case 16:
            write_data(st, "\tInformation Reply\n");
            break;
        case 17:
            write_data(st, "\tAddress Mask Request\n");
            break;
        case 18:
            write_data(st, "\tAddress Mask Reply\n");
            break;
        case 30:
            write_data(st, "\tTraceroute\n");
            break;
        case 42:
            write_data(st, "\tExtended Echo Request\n");
            break;
        case 43:
            write_data(st, "\tExtended Echo Reply\n");
            break;
        default:
            write_data(st, "\tReserved or deprecated\n");
            break;
        }

    return 1;
}