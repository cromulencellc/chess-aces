#include "status.hpp"
#include <sstream>
using namespace response;
std::string Status::name() const {
  switch (_code) {
  case 200:
    return "OK";
  default:
    return std::to_string(_code);
  }
}
int Status::code() const { return _code; }
std::string to_string(const response::Status &s) {
  std::stringstream buf;
  buf << s;
  return buf.str();
}
std::ostream &operator<<(std::ostream &o, const response::Status &s) {
  o << s.code() << " " << s.name();
  return o;
}
