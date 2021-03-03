#include "target.hpp"
#include <sstream>
using namespace request;
Target::Target(Reader &in) {
  std::string p = "";
  std::string q = "";
  char next_char;
  while (in.more()) {
    next_char = in.get();
    if (' ' == next_char)
      break;
    if ('?' == next_char) {
      _has_query = true;
      break;
    }
    p.push_back(next_char);
    if (p.size() > max_path_len) {
      _valid = false;
      path = "";
      query = "";
      return;
    }
  }
  if ('?' == next_char) {
    while (in.more()) {
      next_char = in.get();
      if (' ' == next_char)
        break;
      q.push_back(next_char);
      if (q.size() > max_query_len) {
        _valid = false;
        path = "";
        query = "";
        return;
      }
    }
  }
  path = {p};
  query = q;
}
bool Target::valid() { return _valid; }
std::string to_string(const Target &targ) {
  std::stringstream buf;
  buf << targ;
  return buf.str();
}
