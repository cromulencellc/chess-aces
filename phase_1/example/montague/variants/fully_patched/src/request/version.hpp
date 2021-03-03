#pragma once
#include <istream>
#include <string>
#include "reader.hpp"
namespace request {
class Version {
public:
  static constexpr std::size_t max_version_len = 1 << 4;
  Version(){};
  Version(std::string v) : value(v){};
  Version(Reader &in);
  bool valid();
  friend std::ostream &operator<<(std::ostream &o,
                                  const request::Version &vers) {
    o << "Version(" << vers.value << ")";
    return o;
  }
private:
  std::string value;
};
}
std::string to_string(const request::Version &vers);
