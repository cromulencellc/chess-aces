#include "headers.hpp"

using namespace http;
using namespace std::literals::string_literals;

std::ostream& operator<<(std::ostream& o, const Headers& hs) {
  for (const Header& h : hs) {
    o << h.first << ": "s << h.second << "\r\n"s;
  }
  return o;
}
