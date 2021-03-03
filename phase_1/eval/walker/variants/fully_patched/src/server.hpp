#ifndef __SERVER_HPP__
#define __SERVER_HPP__
#include <algorithm>
#include <array>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "client.hpp"
#include "utils.hpp"
#define SUCCESS 1
#define FAIL 0
#define MAXHNLEN 128
class Server {
  int port;
  int serverfd;
  std::string serverHostname;
  std::vector<Client *> clients;
  std::vector<Client *> toread;
  int initializeSocket();
  int Poll(void);
  int ResolvePolls(void);
  int ParseReads(void);
  int ResolveReads(void);
  int ResolveResponses(void);
  int AddClient(int clientfd, std::string ip);
  int SendBanner(int fd);
  int HandleCommand(std::vector<std::string> tokens, Client *c);
  int InitLoggedInUser(Client *c);
  int ListMaildirs(Client *c, mailbox *parent, std::regex r);
  int ListMaildirs(Client *c, mailbox *parent, std::string s);
  int HandleCapability(std::vector<std::string> tokens, Client *c,
                       std::string uid);
  int HandleNoop(std::vector<std::string> tokens, Client *c, std::string uid);
  int HandleLogout(std::vector<std::string> tokens, Client *c, std::string uid);
  int HandleAuthenticate(std::vector<std::string> tokens, Client *c,
                         std::string uid);
  int HandleLogin(std::vector<std::string> tokens, Client *c, std::string uid);
  int HandleList(std::vector<std::string> tokens, Client *c, std::string uid);
  int HandleCreate(std::vector<std::string> tokens, Client *c, std::string uid);
  int HandleDelete(std::vector<std::string> tokens, Client *c, std::string uid);
  int HandleRename(std::vector<std::string> tokens, Client *c, std::string uid);
  int HandleSelect(std::vector<std::string> tokens, Client *c, std::string uid);
  int HandleExpunge(std::vector<std::string> tokens, Client *c,
                    std::string uid);
  int HandleStatus(std::vector<std::string> tokens, Client *c, std::string uid);
  int HandleClose(std::vector<std::string> tokens, Client *c, std::string uid);
  int HandleFetch(std::vector<std::string> tokens, Client *c, std::string uid);
  int HandleCopy(std::vector<std::string> tokens, Client *c, std::string uid);
  int HandleStore(std::vector<std::string> tokens, Client *c, std::string uid);
public:
  Server(int port);
  void SetPort(int port);
  int GetPort(void);
  void SetFd(int serverfd);
  int GetFd(void);
  void SetServerHostname(std::string serverHostname);
  std::string GetServerHostname(void);
  int Run(void);
};
#endif