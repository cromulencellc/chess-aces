#ifndef __UTILS_HPP__
#define __UTILS_HPP__
#include <iostream>
#include <vector>
#include <sys/mman.h>
#define PF_R 0x04
#define PF_W 0x02
#define PF_X 0x01
bool cppstrncasecmp(const std::string &a, const std::string &b);
uint32_t make_prot(uint32_t flags);
std::vector<std::string> cpptok(std::string base, char c);
#endif