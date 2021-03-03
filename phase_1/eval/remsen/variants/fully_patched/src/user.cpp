#include "user.hpp"
std::vector<ftpuser *> users_g;
std::string userlist;
ftpuser::ftpuser(std::string u, std::string p, std::string homedir,
                 std::string perms) {
  this->username = u;
  this->password = p;
  this->homedir = homedir;
  this->perms = std::stoul(perms, nullptr, 16);
}
ftpuser::ftpuser(ftpuser &src) {
  this->username = src.getuser();
  this->password = src.getpass();
  this->homedir = src.gethomedir();
  this->perms = src.getperms();
  return;
}
std::string ftpuser::getuser(void) { return this->username; }
std::string ftpuser::gethomedir(void) { return this->homedir; }
unsigned long ftpuser::getperms(void) { return this->perms; }
void ftpuser::setperms(unsigned long perms) { this->perms = perms; }
void ftpuser::addperm(unsigned long p) { this->perms |= p; }
unsigned long ftpuser::checkperm(unsigned long p) { return this->perms & p; }
std::string ftpuser::getpass(void) { return this->password; }
int ftpuser::isyou(std::string u) {
  if (u == this->username) {
    return 1;
  }
  return 0;
}
int ftpuser::goodpass(std::string p) {
  if (p == this->password) {
    return 1;
  }
  return 0;
}
int parse_userlist(std::string filename) {
  std::string line;
  std::ifstream infile(filename);
  std::vector<std::string> tokline;
  ftpuser *user_c = NULL;
  if (!infile.is_open()) {
    std::cout << "[ERROR] Failed to open: " << filename << std::endl;
    return FAIL;
  }
  while (getline(infile, line)) {
    tokline = cpptok(line, ':');
    if (tokline.size() != 4) {
      std::cout << "[ERROR] Invalid file" << std::endl;
      infile.close();
      return FAIL;
    }
    user_c = new ftpuser(tokline[0], tokline[1], tokline[2], tokline[3]);
    if (!user_c) {
      std::cout << "Failed to allocate new user" << std::endl;
      return FAIL;
    }
    users_g.push_back(user_c);
  }
  infile.close();
  return SUCCESS;
}
void print_userlist() {
  ftpuser *tu = NULL;
  for (std::vector<ftpuser *>::iterator it = users_g.begin();
       it != users_g.end(); ++it) {
    tu = *it;
    if (tu) {
      std::cout << tu->getuser() << std::endl;
      std::cout << tu->getpass() << std::endl;
    }
  }
}