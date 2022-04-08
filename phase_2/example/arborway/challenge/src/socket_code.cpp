#include "socket_code.hpp"

/// Starts a listener for a socket. Returns 0 on failure
int setup_socket( int port )
{
    int sockfd = 0;
    struct sockaddr_in srvr;
    size_t enable = 1;

    sockfd = socket( AF_INET, SOCK_STREAM, 0);

    if ( sockfd < 0 ) {
        std::cout << "socket() failed: " << strerror(errno) << std::endl;
        return 0;
    }

    memset( &srvr, 0, sizeof(struct sockaddr_in));

    srvr.sin_family = AF_INET;
    srvr.sin_addr.s_addr = INADDR_ANY;
    srvr.sin_port = htons(port);

    // Bind the socket to the port
    if ( bind( sockfd, (struct sockaddr *)&srvr, sizeof(srvr) ) < 0 ) {
        std::cout << "bind() failed: " << strerror(errno) << std::endl;
        close(sockfd);
        return 0;
    }

    if ( setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(enable)) < 0 ) {
        std::cout << "setsockopt() failed: " << strerror(errno) << std::endl;
        close(sockfd);
        return 0;
    }

    if ( listen(sockfd, 1024 ) < 0 ) {
        std::cout << "listen() failed: " << strerror(errno) << std::endl;
        close(sockfd);
        return 0;
    }

    std::cout << "[INFO] Listener socket on port: " << port << std::endl;

    return sockfd;
}

int accept_socket( int fd )
{
    int connfd = 0;
    struct sockaddr_in ca;
    socklen_t ca_len = sizeof(ca);

    connfd = accept( fd, (struct sockaddr *)&ca, &ca_len);

    if ( connfd < 0 ) {
        std::cout << "accept() failed: " << strerror(errno) << std::endl;
        return 0;
    }

    std::cout << "[INFO] Accepted incoming connection" << std::endl;
    
    close(fd);

    return connfd;
}

/// Prompts a user for a file size then reads in that length.
/// It sets data to a pointer of the specified length and returns the size
/// Returns 0 on failure
int readfile( int fd, char**data)
{
    uint64_t length;
    uint64_t result;
    uint64_t bytes_read = 0;

    char *tempdata = NULL;

    if ( data == NULL ) {
        return 0;
    }

    std::cout << "Enter the file size: ";
    std::cin >> length;

    if ( length > 2000000) {
        std::cout << "Too big.." << std::endl;
        return 0;
    }

    tempdata = (char*)malloc( length);

    if ( tempdata == NULL ) {
        return 0;
    }

    std::cout << "Send the file in 1000 byte chunks" << std::endl;

    while ( bytes_read < length ) {
        result = ( length - bytes_read ) >= 1000 ? 1000 : length - bytes_read;

        result = read( fd, tempdata + bytes_read, result);

        if ( result <= 0 ) {
            std::cerr << "Read failed: " << bytes_read << " -- " << length << std::endl;
            free(tempdata);
            return 0;
        }

        bytes_read += result;

        if ( bytes_read < length ) {
            write(fd, "next\n", 5);
        }
    }

    write(fd, "done\n", 5);

    *data = tempdata;

    return bytes_read;
}


                    