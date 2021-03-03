#ifndef __USERCLASS__
#define __USERCLASS__
#include <fstream>
#include <iostream>
#include <vector>
#include "utils.hpp"
class ftpuser {
  std::string username;
  std::string password;
  std::string homedir;
  unsigned long perms;
public:
  ftpuser(std::string username, std::string password, std::string homedir,
          std::string perms);
  ftpuser(ftpuser &src);
  std::string getuser(void);
  std::string getpass(void);
  std::string gethomedir(void);
  unsigned long getperms(void);
  void setperms(unsigned long);
  void addperm(unsigned long);
  unsigned long checkperm(unsigned long);
  int isyou(std::string u);
  int goodpass(std::string p);
};
int parse_userlist(std::string filename);
void print_userlist();
#endif