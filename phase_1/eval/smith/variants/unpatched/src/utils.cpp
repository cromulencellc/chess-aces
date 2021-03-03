#include "utils.hpp"
std::vector<std::string> tokenize_line(std::string line, char c) {
  std::stringstream ss(line);
  std::vector<std::string> tokens;
  std::string imm;
  while (getline(ss, imm, c)) {
    tokens.push_back(imm);
  }
  return tokens;
}
void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](int ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}
bool cppstrncasecmp(const std::string &a, const std::string &b) {
  return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) {
    return std::tolower(a) == std::tolower(b);
  });
}
int makeresponse(char *dest, const char *format, ...) {
  int length;
  va_list args;
  va_start(args, format);
  length = vsprintf(dest, format, args);
  va_end(args);
  return length;
}