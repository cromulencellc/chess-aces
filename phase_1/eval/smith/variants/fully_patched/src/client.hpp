#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__
#include <iostream>
#include <regex>
#include <stdio.h>
#include <unistd.h>
#include "utils.hpp"
#define FAIL 0
#define SUCCESS 1
#define OPERATOR 1
typedef struct modes {
  std::string channel;
  int flags;
} modes;
class Client {
  std::string user;
  std::string nick;
  std::string hostname;
  std::time_t connect_time;
  std::time_t last_msg;
  std::vector<std::string> invites_to;
  std::vector<std::string> silenced;
  int fd;
  int NeedsShutdown;
  int needsResolving;
  std::string Queue;
  int IsRegistered;
  std::vector<std::string> ResponseQueue;
  int NickSet;
  int UserSet;
  int pingsent;
  int mode;
  int caps;
  int away;
  std::string awaymsg;
  time_t lastknocktime;

public:
  Client(int fd);
  ~Client();
  void SetAwayMsg(std::string msg);
  std::string GetAwayMsg(void);
  int GetAway(void);
  void SetAway(int flag);
  void SetUser(std::string user);
  std::string GetUser(void);
  int IsSilenced(std::string nick);
  void AddSilenced(std::string nick);
  void RemoveSilenced(std::string nick);
  void SetConnectTime(std::time_t connect_time);
  std::time_t GetConnectTime(void);
  void SetLastKnockTime(std::time_t knocktime);
  std::time_t GetLastKnockTime(void);
  void SetLastMsg(std::time_t last_msg);
  std::time_t GetLastMsg(void);
  void SetNick(std::string nick);
  std::string GetNick(void);
  void SetHostname(std::string hostname);
  std::string GetHostname(void);
  void AddInviteTo(std::string channel);
  int IsInvited(std::string channel);
  int GetFd(void);
  void SetFd(int fd);
  void SetNeedsShutdown(int NeedsShutdown);
  int GetNeedsShutdown(void);
  void SetPingSent(int flag);
  int GetPingSent(void);
  void SetNeedsResolving(int needsResolving);
  int GetNeedsResolving(void);
  void SetQueue(std::string Queue);
  std::string GetQueue(void);
  void SetRegistered(int IsRegistered);
  int GetRegistered(void);
  int ShutdownClient(void);
  int ClearQueue(void);
  int Read(void);
  int Write(std::string line);
  int SendResponseQueue();
  int AddLineToResponseQueue(char *line);
  int AddLineToResponseQueue(std::string line);
  int NickUserSet();
  int IsNickSet(void);
  void SetCapFlag(int flag);
  int GetCapFlag(int flag);
  int CapSet(int flag);
  void RemoveCap(int flag);
};
#endif