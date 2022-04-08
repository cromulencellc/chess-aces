#ifndef __SOCKET_CODE_HPP__
#define __SOCKET_CODE_HPP__
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
int setup_socket(int port);
int accept_socket(int fd);
int readfile(int fd, char **data);
#endif