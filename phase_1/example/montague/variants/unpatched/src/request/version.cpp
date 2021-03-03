#include "version.hpp"
#include <sstream>
using namespace request;
Version::Version(Reader &in) {
  std::string v = "";
  char next_char;
  while (in.more()) {
    next_char = in.get();
    if ('\r' == next_char) {
      in.push_back('\r');
      break;
    }
    v.push_back(next_char);
    if (v.size() > max_version_len) {
      v = "";
      return;
    }
  }
  value = v;
}
std::string to_string(const request::Version &vers) {
  std::stringstream buf;
  buf << vers;
  return buf.str();
}
