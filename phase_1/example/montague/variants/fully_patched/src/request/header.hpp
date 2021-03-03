#pragma once
#include <istream>
#include <string>
#include <utility>
#include "reader.hpp"
namespace request {
using _Header_base_class = std::pair<const std::string, std::string>;
class Header : public _Header_base_class {
public:
  static Header parse(Reader &in);
  Header(bool invalid) : valid(false){};
  Header(std::string key, std::string value)
      : _Header_base_class(key, value), valid(true){};
  Header(const _Header_base_class hbc) : _Header_base_class(hbc){};
  bool valid = false;
};
}
std::ostream &operator<<(std::ostream &o, request::Header &h);
