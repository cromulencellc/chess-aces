#ifndef __SOCKET_CODE_HPP__
#define __SOCKET_CODE_HPP__

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

int setup_socket( int port );
int accept_socket( int fd );
int readfile( int fd, char **data );
#endif