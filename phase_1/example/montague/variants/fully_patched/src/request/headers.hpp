#pragma once
#include <map>
#include <string>
#include "header.hpp"
#include "reader.hpp"
namespace request {
using _Headers_base_class = std::map<std::string, std::string>;
class Headers : public _Headers_base_class {
public:
  Headers(Reader &in);
  Headers() : _Headers_base_class{} {};
  bool valid = false;
};
}
std::ostream &operator<<(std::ostream &o, request::Headers &hs);
