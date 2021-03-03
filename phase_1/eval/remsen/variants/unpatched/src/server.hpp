#ifndef __SERVER_HPP__
#define __SERVER_HPP__
#include <algorithm>
#include <array>
#include <iostream>
#include <poll.h>
#include "connection.hpp"
class Server {
  int port;
  struct sockaddr_in srvr;
  int sockfd;
  fd_set readfds;
  int stdin_only;
  std::vector<connection *> clients;
  std::vector<connection *> poll_list;
  std::vector<connection *> to_remove;

public:
  Server(int port);
  Server();
  int setup_socket();
  int select_loop();
  int SetFds();
  virtual int AddSingleShotRETR(int fd, int controlfd, char *buffer, int length,
                                std::string fp, std::string success,
                                int needsAccept);
  virtual int AddConnection(int fd, sockaddr_in *clnt);
  virtual int Poll();
  virtual int ResolvePolls();
  virtual int ResolveCommands();
  virtual int ResolveShutdown();
};
#endif