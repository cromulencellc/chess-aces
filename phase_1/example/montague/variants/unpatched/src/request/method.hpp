#pragma once
#include <istream>
#include <string>
#include "reader.hpp"
namespace request {
class Method {
public:
  static constexpr std::size_t max_method_name_len = 7;
  enum class Name {
    _INVALID,
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE
  };
  static Method parse(Reader &in);
  Method(){};
  Method(std::string candidate_name);
  Method(const Method &other) : name(other.name){};
  bool valid();
  Name name = Name::_INVALID;
};
}
std::string to_string(request::Method::Name meth);
std::ostream &operator<<(std::ostream &os, request::Method::Name meth);
std::string to_string(request::Method meth);
std::ostream &operator<<(std::ostream &os, request::Method &meth);
