#ifndef __UTILS_HPP__
#define __UTILS_HPP__
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
std::vector<std::string> tokenize_line(std::string, char c);
void rtrim(std::string &s);
bool cppstrncasecmp(const std::string &a, const std::string &b);
int makeresponse(char *dest, const char *format, ...);
#endif