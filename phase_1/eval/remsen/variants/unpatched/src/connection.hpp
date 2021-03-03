#ifndef __CONNECTION_HPP__
#define __CONNECTION_HPP__
#include <iostream>
#include <string>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <numeric>
#include <regex>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "user.hpp"
#include "utils.hpp"
class connection {
  int fd;
  int needs_shutdown;
  int command_in_queue;
  int userset;
  int passset;
  unsigned int clientid;
  int mode;
  int authd;
  struct sockaddr_in clnt;
  std::string Command;
  std::vector<std::string> command_tokens;
  std::string rnfr;
  int rnfrset;
  ftpuser *userclass;
  std::string user;
  std::string pass;
  int islocal;
  unsigned int restvalue;
  std::string cwd;
  char structtype;
  int portset;
  std::string targetip;
  int targetport;
  int needs_accept;
  char *data_to_send;
  int datalength;
  int single_use;
  std::string success;
  std::string outfile;
  int controlfd;
  int pasv;
  int pasvfd;
public:
  connection(int fd, sockaddr_in *clnt);
  int getIsLocal(void);
  void setNeedsAccept(int flag);
  int getNeedsAccept(void);
  void setSuccessString(std::string success);
  std::string getSuccessString(void);
  void setOutfile(std::string outfile);
  std::string getOutfile(void);
  void setControlfd(int fd);
  int getControlfd(void);
  void setDataToSend(char *data);
  char *getDataToSend(void);
  void setDataLength(int length);
  int getDataLength(void);
  void setSingleUse(int flag);
  int getSingleUse(void);
  int get_fd(void);
  void setFD(int fd);
  int GetMode(void);
  void SetMode(int mode);
  void SetStructType(char st);
  char GetStructType(void);
  unsigned int GetRestValue(void);
  void SetRestValue(unsigned int rv);
  int Read(void);
  int Write(std::string data);
  int NeedsRemoval(void);
  int SendBanner(void);
  int CommandWaiting(void);
  int ParseCommand(void);
  int HandleCommand(void);
  void SetUser(std::string user);
  void SetPass(std::string pass);
  int CheckAuth();
  std::string GetCwd(void);
  void SetCwd(std::string cwd);
  void FlushToSend(void);
  void AcceptToSend(void);
  void FlushFromRead(void);
  void AcceptToRead(void);
  void AcceptToAppend(void);
  void FlushFromAppend(void);
  int HandleABOR(void);
  int HandleACCT(void);
  int HandleALLO(void);
  int HandleAPPE(void);
  int HandleCDUP(void);
  int HandleCWD(void);
  int HandleDELE(void);
  int HandleEPRT(void);
  int HandleEPSV(void);
  int HandleFEAT(void);
  int HandleHELP(void);
  int HandleLIST(void);
  int HandleMDTM(void);
  int HandleMKD(void);
  int HandleMODE(void);
  int HandleNLST(void);
  int HandleNOOP(void);
  int HandleOPTS(void);
  int HandlePASV(void);
  int HandlePORT(void);
  int HandlePWD(void);
  int HandleREIN(void);
  int HandleREST(void);
  int HandleRETR(void);
  int HandleRMD(void);
  int HandleRNFR(void);
  int HandleRNTO(void);
  int HandleSITE(void);
  int HandleSIZE(void);
  int HandleSTAT(void);
  int HandleSTOR(void);
  int HandleSTOU(void);
  int HandleSTRU(void);
  int HandleSYST(void);
  int HandleTYPE(void);
  int HandleXCUP(void);
  int HandleXCWD(void);
  int HandleXMKD(void);
  int HandleXPWD(void);
  int HandleXRMD(void);
};
#endif