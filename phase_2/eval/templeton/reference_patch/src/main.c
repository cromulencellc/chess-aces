#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "socket_code.h"
#include "heap.h"
#include "http.h"

void takeover( int fd )
{
    close(fileno(stdin));
    close(fileno(stdout));

    dup2( fd, 0);
    dup2( fd, 1);

    return;
}

char * readtoken( char *data )
{
        struct stat st;
        int fd;

        if ( data == NULL ) {
        	exit(1);
        }

        if ( stat("/token", &st) != 0 ) {
                printf("[ERROR} failed to stat token\n");
                exit(1);
        }

        fd = open( "/token", O_RDONLY);

        if ( fd <= 0 ) {
                printf("[ERROR] failed to open token: %s\n", strerror(errno));
                exit(1);
        }

        strcpy(data, "token:  ");

        if ( read( fd, data + 8, st.st_size) <= 0 ) {
            exit(1);
        }

        close(fd);

        return data;
}

void usage( char *nm )
{
    if ( !nm ) {
        exit(1);
    }

    printf("USAGE: %s -s\n", nm);
    printf("\t-s -- Tells templeton to use stdin/stdout for input and output.\n");
    printf("\tIf -s is not specified then templeton attempts to use the port specified by the\n");
    printf("\tPORT environment variable.\n");

    exit(1);
}

void readloop( )
{
	char buffer[1024];
	int nbytes = 0;
    int total = 0;

	while ( 1 ) {
		memset(buffer, 0, 1024);

        total = 0;

        while ( total < 1024 && strstr(buffer, "\r\n\r\n") == NULL ) {
            nbytes = read( fileno(stdin), buffer, sizeof(buffer)-1);

            if ( nbytes <= 0 ) {
                fprintf(stderr, "[INFO] Client disconnected\n");
                return;
            }
        }

        handle_http_request(buffer);    
	}
}

int main( int argc, char **argv, char **envp)
{
    uint64_t stdin_flag = 0;
    uint64_t c;
    uint64_t port = 0;
    uint64_t fd = 0;
    uint64_t s_fd = 0;
    char token[128] = {0};

    #ifndef NO_TESTBED
    char *b = getenv("CHESS");
    if ( b == NULL ) {
    	printf("[TESTBED] ENV variable check failed\n");
        exit(0);
    }
    #endif
    
    if ( init_heap() ) {
    	exit(1);
    }

    while ((c = getopt (argc, argv, "s")) != -1) {
        switch (c) {
            case 's':
                stdin_flag = 1;
                break;
            case '?':
            	printf("[ERROR] Unknown option %c\n", optopt);
                usage(argv[0]);
                break;
            default:
                exit(1);
        }
    }

    if ( stdin_flag == 0 ) {
        if ( getenv("PORT") == NULL ) {
            usage(argv[0]);
        }

        port = atoi(getenv("PORT"));

        s_fd = setup_socket( port );

        if ( s_fd == 0) {
            exit(0);
        }

        fd = accept_socket( s_fd );

        if ( fd == 0) {
            exit(0);
        }

        close(s_fd);
        takeover(fd);
    }

    readtoken(token);

    readloop();
    close(fd);
    close(0);
    close(1);

    return 0;
}
