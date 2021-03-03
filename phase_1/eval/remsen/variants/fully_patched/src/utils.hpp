#ifndef __UTILS_HPP__
#define __UTILS_HPP__
#define SUCCESS 0
#define FAIL 1
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <linux/limits.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
std::vector<std::string> cpptok(std::string base, char c);
bool endswith(std::string base, std::string needle);
bool startswith(std::string base, std::string needle);
bool onlydigits(std::string base);
std::string getcwd(void);
int connect_socket(std::string ip, int port);
int read_data(int fd, char **buffer);
std::vector<std::string> get_detailed_listing(std::string directory,
                                              std::string cwd);
std::vector<std::string> get_listing(std::string directory, std::string cwd);
int checkperms(unsigned long perms, unsigned long tocheck);
#define ABOR 0x00000000001
#define ACCT 0x00000000002
#define ALLO 0x00000000004
#define APPE 0x00000000008
#define CDUP 0x00000000010
#define CWD 0x00000000020
#define DELE 0x00000000040
#define EPRT 0x00000000080
#define EPSV 0x00000000100
#define FEAT 0x00000000200
#define HELP 0x00000000400
#define LIST 0x00000000800
#define MDTM 0x00000001000
#define MKD 0x00000002000
#define MODE 0x00000004000
#define NLST 0x00000008000
#define NOOP 0x00000010000
#define OPTS 0x00000020000
#define PASS 0x00000040000
#define PASV 0x00000080000
#define PORT 0x00000100000
#define PWD 0x00000200000
#define QUIT 0x00000400000
#define REIN 0x00000800000
#define REST 0x00001000000
#define RETR 0x00002000000
#define RMD 0x00004000000
#define RNFR 0x00008000000
#define RNTO 0x00010000000
#define SITE 0x00020000000
#define SIZE 0x00040000000
#define SMNT 0x00080000000
#define STAT 0x00100000000
#define STOR 0x00200000000
#define STOU 0x00400000000
#define STRU 0x00800000000
#define SYST 0x01000000000
#define TYPE 0x02000000000
#define USER 0x04000000000
#define XCUP 0x08000000000
#define XCWD 0x10000000000
#define XMKD 0x20000000000
#define XPWD 0x40000000000
#define XRMD 0x80000000000
#define CANLOGINREMOTE 0x100000000000
#define CANRETROUTSIDE 0x200000000000
#define CANRETRTOKEN 0x400000000000
#define CANRETRPLIST 0x800000000000
#define ASCII 0
#define BINARY 1
#endif