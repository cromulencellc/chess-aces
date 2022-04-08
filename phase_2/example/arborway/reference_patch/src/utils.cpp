#include "utils.hpp"
bool cppstrncasecmp(const std::string &a, const std::string &b) {
  return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) {
    return std::tolower(a) == std::tolower(b);
  });
}
uint32_t make_prot(uint32_t flags) {
  uint32_t prot = 0;
  if (flags & PF_R) {
    prot |= PROT_READ;
  }
  if (flags & PF_W) {
    prot |= PROT_WRITE;
  }
  if (flags & PF_X) {
    prot |= PROT_EXEC;
  }
  return prot;
}
std::vector<std::string> cpptok(std::string base, char c) {
  std::vector<std::string> result;
  std::string newcut;
  std::size_t end = 0;
  std::size_t start = 0;
  while (end != std::string::npos) {
    end = base.find(c, start);
    if (end == std::string::npos) {
      newcut = base.substr(start, base.size() - start);
    } else {
      newcut = base.substr(start, end - start);
    }
    result.push_back(newcut);
    start = end + 1;
  }
  return result;
}