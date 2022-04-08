#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int setup_socket( int port )
{
    int sockfd = 0;
    struct sockaddr_in srvr;
    size_t enable = 1;

    sockfd = socket( AF_INET, SOCK_STREAM, 0);

    if ( sockfd < 0 ) {
    	printf("socket() failed: %s\n", strerror(errno));
        return 0;
    }

    memset( &srvr, 0, sizeof(struct sockaddr_in));

    srvr.sin_family = AF_INET;
    srvr.sin_addr.s_addr = INADDR_ANY;
    srvr.sin_port = htons(port);

    if ( bind( sockfd, (struct sockaddr *)&srvr, sizeof(srvr) ) < 0 ) {
    	printf("bind() failed: %s\n", strerror(errno));
        close(sockfd);
        return 0;
    }

    if ( setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(enable)) < 0 ) {
    	printf("setsockopt() failed: %s\n", strerror(errno));
        close(sockfd);
        return 0;
    }

    if ( listen(sockfd, 1024 ) < 0 ) {
    	printf("listen() failed: %s\n", strerror(errno));
        close(sockfd);
        return 0;
    }

    printf("[INFO] Listener socket on port: %d\n", port);

    return sockfd;
}

int accept_socket( int fd )
{
    int connfd = 0;
    struct sockaddr_in ca;
    socklen_t ca_len = sizeof(ca);

    connfd = accept( fd, (struct sockaddr *)&ca, &ca_len);

    if ( connfd < 0 ) {
    	printf("accept() failed: %s\n", strerror(errno));
        return 0;
    }

    printf("[INFO] Accepted incoming connection\n");
    
    close(fd);

    return connfd;
}