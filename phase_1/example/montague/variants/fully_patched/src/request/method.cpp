#include "method.hpp"
#include <string>
using namespace request;
Method Method::parse(Reader &in) {
  std::string running = "";
  Method current;
  while (in.more()) {
    char next_letter = in.get();
    if (' ' == next_letter) {
      return current;
    }
    running.push_back(next_letter);
    current = {running};
    if (running.size() > max_method_name_len) {
      return {};
    }
  }
  return {};
}
Method::Method(std::string candidate_name) {
  if ("GET" == candidate_name) {
    name = Name::GET;
    return;
  }
  if ("HEAD" == candidate_name) {
    name = Name::HEAD;
    return;
  }
  if ("POST" == candidate_name) {
    name = Name::POST;
    return;
  }
  if ("PUT" == candidate_name) {
    name = Name::PUT;
    return;
  }
  if ("DELETE" == candidate_name) {
    name = Name::DELETE;
    return;
  }
  if ("CONNECT" == candidate_name) {
    name = Name::CONNECT;
    return;
  }
  if ("OPTIONS" == candidate_name) {
    name = Name::OPTIONS;
    return;
  }
  if ("TRACE" == candidate_name) {
    name = Name::TRACE;
    return;
  }
  name = Name::_INVALID;
}
bool Method::valid() {
  if (Name::_INVALID == name)
    return false;
  return true;
}
std::string to_string(Method meth) { return to_string(meth.name); }
std::string to_string(Method::Name meth) {
  switch (meth) {
  case Method::Name::_INVALID:
    return "_INVALID";
  case Method::Name::GET:
    return "GET";
  case Method::Name::HEAD:
    return "HEAD";
  case Method::Name::POST:
    return "POST";
  case Method::Name::PUT:
    return "PUT";
  case Method::Name::DELETE:
    return "DELETE";
  case Method::Name::CONNECT:
    return "CONNECT";
  case Method::Name::OPTIONS:
    return "OPTIONS";
  case Method::Name::TRACE:
    return "TRACE";
  default:
    return "unknown method";
  }
}
std::ostream &operator<<(std::ostream &os, Method::Name meth) {
  os << to_string(meth);
  return os;
}
std::ostream &operator<<(std::ostream &os, Method &meth) {
  os << meth.name;
  return os;
}
