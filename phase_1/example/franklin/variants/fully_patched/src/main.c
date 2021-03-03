#include <stdio.h>
#include "ethernet.h"
#include "ipheader.h"
#include "stream.h"
#include "udp.h"
#include "tcp.h"
#include "icmp.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "testbed.h"

typedef struct packet {
    eth *eh;
    union {
        iphv4 *ipv4_h;
        iphv6 * ipv6_h;
    } ip_header;
    uint16_t upper_protocol;    

    udp *udph;
    tcp *tcph;
} packet;

int port = 3004;

stream *st = NULL;

int accept_loop( int fd )
{
    int conn_fd = 0;
    uint32_t length;
    uint8_t *data;
    struct sockaddr_in ca;
    packet p;

    socklen_t ca_len = sizeof( struct sockaddr_in );
    memset(&ca, 0, sizeof(struct sockaddr_in));

    while ( 1 ) {
        conn_fd = accept( fd, (struct sockaddr *)&ca, &ca_len);

        if ( conn_fd < 0 ) {
            close(fd);
            return -1;
        }

        if ( read( conn_fd, &length, 4 ) <= 0 ) {
            close(conn_fd);
            close(fd);
            return -1;
        }

        if ( length > 1024 ) {
            write( conn_fd, "[ERROR] Too big\n", 16);
            close(conn_fd);
            close(fd);
            return -1;
        }

        data = malloc( length );

        if ( !data ) {
            close(conn_fd);
            close(fd);
            return -1;
        }

        if ( read( conn_fd, data, length ) != length ) {
            write( conn_fd, "[ERROR] Failed to read\n", 23);
            close(conn_fd);
            close(fd);
            free(data);
            return -1;
        }

        st = init_stream( data, length );

        free(data);

        if ( !st ) {
            close(conn_fd);
            close(fd);
            return -1;
        }

        set_fd( st, conn_fd );

        write_data( st, "Begin parsing...\n");

        p.eh = parse_ethernet_header( st ) ;

        exit(0);
    }
    return 0;
}

void usage( char *pn )
{
    fprintf(stderr, "USAGE: %s -p <packet_file> -s\n", pn);
    fprintf(stderr, "-s Used to specify that the CB will use stdin/stdout. This option requires the -p option\n");
    fprintf(stderr, "-p Indicate the raw packet file to be opened and parsed\n");

    exit(1);
}

int setup_socket( int port )
{
    int fd;
    struct sockaddr_in sa;
    int enable = 1;
    
    fd = socket( AF_INET, SOCK_STREAM, 0);

    if ( fd < 0 ) {
        fprintf(stderr, "[ERROR] 1\n");
        exit(0);
    }

    memset( &sa, 0, sizeof(struct sockaddr_in));

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(port);

    
    if ( bind( fd, (struct sockaddr *)&sa, sizeof(sa) ) < 0 ) {
        fprintf(stderr, "Error on binding\n");
        close(fd);
        return -1;
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0 ) {
        fprintf(stderr, "Error on setsockopt\n");
        close(fd);
        return -1;
    }

    if ( listen(fd, 0 ) < 0 ) {
        fprintf(stderr, "Error on listen\n");
        close(fd);
        return -1;
    }

    fprintf(stderr, "[INFO] Listener socket on port: %d\n", port);

    return fd;
}

int main(int argc, char **argv)
{
    packet p;
    char *packet_file = NULL;
    int s = 0;
    char c;
    int fd;

    #ifndef NO_TESTBED
    assert_execution_on_testbed();
    #endif

    char *t = getenv("PORT");

    if ( t != NULL ) {
        port = atoi(t);
    } else {
        port = 3004;
    }

    while ((c = getopt (argc, argv, "p:s")) != -1)
    switch (c)
      {
      case 'p':
        if ( packet_file ) {
            fprintf(stderr, "Only one -p option permitted\n");
            usage(argv[0]);
        }

        packet_file = strdup(optarg);

        if ( packet_file == NULL ) {
            return 1;
        }

        break;
      case 's':
        s = 1;
        break;
      case '?':
        if (optopt == 'p') {
          fprintf(stderr, "-%c argument required\n", optopt);
          usage(argv[0]);
        }
        else {
            fprintf(stderr, "Unknown option\n");
            usage(argv[0]);
        }
      default:
        abort ();
      }

    if ( !s ) {
        fd = setup_socket( port );

        if ( fd < 0 ) {
            exit(1);
        }

        accept_loop( fd );
    } else {
        st = open_packet_file( packet_file );

        if ( !st ) {
            exit(1);
        }

        set_fd( st, fileno(stdout) );

        p.eh = parse_ethernet_header( st ) ;
    }

    

    return 0;
}