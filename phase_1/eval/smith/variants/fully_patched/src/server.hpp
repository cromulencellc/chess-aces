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
#include "channel.hpp"
#include "client.hpp"
#include "utils.hpp"
#define MAXHNLEN 128
#define MAXINCHAN 15
#define MAXNICKLEN 30
#define MAXUSERLEN 30
#define CHANNELLEN 50
#define MAXAWAYMSGLEN 64
#define MAXPRIVMSGLEN 256
#define MAXTOPICLEN 128
#define MAXCAPLEN 20
#define SUCCESS 1
#define FAIL 0
#define CAP_MULTI 1
#define CAP_AWAYN 2
class Server {
  int port;
  int serverfd;
  std::string serverHostname;
  std::string logfile;
  int logging;
  std::vector<Client *> clients;
  std::vector<Client *> toread;
  std::vector<Channel *> channels;
  std::vector<std::string> tokens;
  Client *c;
  int initializeSocket();
  void WriteLogEntry(std::string log);
  int Poll(void);
  int ResolvePolls(void);
  int ParseReads(void);
  int HandleCommand();
  int HandleAWAY();
  int HandleCAP();
  int HandleINVITE();
  int HandleISON();
  int HandleJOIN();
  int HandleKICK();
  int HandleKNOCK();
  int HandleLIST();
  int HandleMODE();
  int HandleMOTD();
  int HandleNAMES();
  int HandleNICK();
  int HandleNOTICE();
  int HandlePART();
  int HandlePING();
  int HandlePRIVMSG();
  int HandleQUIT();
  int HandleSILENCE();
  int HandleTIME();
  int HandleTOPIC();
  int HandleUSER();
  int HandleWHO();
  int HandleWHOIS();
  int ResolveReads(void);
  int ResolveResponses(void);
  int AddClient(int clientfd, std::string ip);
  int SendBanner(int fd);
  int NickExists(std::string nick);
  int RemoveClientByNick(std::string nick);
  Channel *GetChannelByName(std::string name);
  Client *GetClientByNick(std::string nick);
  int SendServerInfo(Client *c);
  int CheckIdles();
  int CleanupChannels();
public:
  Server(int port);
  void SetPort(int port);
  int GetPort(void);
  void SetFd(int serverfd);
  int GetFd(void);
  void SetLogfile(std::string logfile);
  std::string GetLogfile(void);
  void SetLogging(int logging);
  int GetLogging(void);
  void SetServerHostname(std::string serverHostname);
  std::string GetServerHostname(void);
  int Run(void);
};
#endif