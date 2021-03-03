#ifndef __UTILS_HPP__
#define __UTILS_HPP__
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
std::vector<std::string> tokenize_line(std::string, char c);
void rtrim(std::string &s);
bool cppstrncasecmp(const std::string &a, const std::string &b);
int base64encode(const char *data_in, char *data_out, int in_len, int max_out);
int base64decode(const char *data_in, char *data_out, int in_len, int max_out);
std::vector<std::string> get_file_list(std::string fullpath);
bool isnumber(const std::string &s);
#endif