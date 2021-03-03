#pragma once
#include <ostream>
#include <string>
namespace response {
class Status {
public:
  Status(int c) : _code(c){};
  std::string name() const;
  int code() const;
private:
  int _code;
};
}
std::ostream &operator<<(std::ostream &o, const response::Status &s);
std::string to_string(const response::Status &s);
