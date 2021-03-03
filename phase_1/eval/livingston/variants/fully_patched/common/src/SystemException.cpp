#include "SystemException.h"
#include <execinfo.h>
#include <sstream>
#define STACK_RESOLTION (1000)
SystemException::SystemException(std::string &message) {
  std::stringstream ss;
  ss << message << std::endl;
  void *frames[STACK_RESOLTION];
  int numFrames = backtrace(frames, STACK_RESOLTION);
  char **frameNames = backtrace_symbols(frames, numFrames);
  if (frameNames) {
    for (int x = 0; x < numFrames; ++x) {
      ss << frameNames[x] << std::endl;
    }
  }
  data = ss.str();
}
SystemException::SystemException(const char *message) {
  std::stringstream ss;
  ss << message << std::endl;
  void *frames[STACK_RESOLTION];
  int numFrames = backtrace(frames, STACK_RESOLTION);
  char **frameNames = backtrace_symbols(frames, numFrames);
  if (frameNames) {
    for (int x = 0; x < numFrames; ++x) {
      ss << frameNames[x] << std::endl;
    }
  }
  data = ss.str();
}
const char *SystemException::what() const throw() { return this->data.c_str(); }
std::ostream &operator<<(std::ostream &out, const SystemException &e) {
  out << e.what();
  return out;
}