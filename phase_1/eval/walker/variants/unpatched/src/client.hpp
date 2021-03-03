#ifndef __CLIENT_HPP__
#define __CLIENT_HPP__
#include <iostream>
#include <regex>
#include <stdio.h>
#include <unistd.h>
#include "mailbox.hpp"
#define FAIL 0
#define SUCCESS 1
class Client {
  int fd;
  int NeedsShutdown;
  int needsResolving;
  std::string Queue;
  std::vector<std::string> ResponseQueue;
  int loggedin;
  std::string maildir;
  mailbox *selected;
  std::string username;
  uid_t uid;
  uid_t gid;
public:
  Client(int fd);
  virtual ~Client();
  virtual int GetFd(void);
  virtual void SetFd(int fd);
  virtual void SetNeedsShutdown(int NeedsShutdown);
  virtual int GetNeedsShutdown(void);
  virtual void SetNeedsResolving(int needsResolving);
  virtual int GetNeedsResolving(void);
  virtual void SetQueue(std::string Queue);
  virtual std::string GetQueue(void);
  virtual void SetLoggedIn(int value);
  virtual int GetLoggedIn(void);
  virtual void SetUid(int value);
  virtual int GetUid(void);
  virtual void SetGid(int value);
  virtual int GetGid(void);
  virtual void SetMailDir(std::string MailDir);
  virtual std::string GetMailDir(void);
  virtual void SetSelected(mailbox *Selected);
  virtual mailbox *GetSelected(void);
  virtual void SetUsername(std::string username);
  virtual std::string GetUsername(void);
  virtual int ShutdownClient(void);
  virtual int ClearQueue(void);
  virtual int Read(void);
  virtual int Write(std::string line);
  virtual int SendResponseQueue();
  virtual int AddLineToResponseQueue(char *line);
  virtual int AddLineToResponseQueue(std::string line);
};
#endif