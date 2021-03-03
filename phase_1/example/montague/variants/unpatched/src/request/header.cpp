#include "header.hpp"
using namespace request;
Header Header::parse(Reader &in) {
  std::string key = in.read_to(':');
  if (!in.expect(':'))
    return {false};
  if (!in.expect(' '))
    return {false};
  std::string value = in.read_to('\r');
  if (!in.expect('\r'))
    return {false};
  if (!in.expect('\n'))
    return {false};
  return {key, value};
}
std::ostream &operator<<(std::ostream &o, request::Header &h) {
  o << h.first << ": " << h.second;
  return o;
}
