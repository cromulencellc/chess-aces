#pragma once
#include "address.hpp"
#include "request/body_parser.hpp"
#include "request/headers.hpp"
#include "request/method.hpp"
#include "request/target.hpp"
#include "request/version.hpp"
#include <optional>
class Request {
public:
  Request(int i, Address c_a);
  bool match(std::string path, request::Method::Name method);
  std::optional<request::body_parser::DecodedForm> decode_body();
  request::Method method;
  request::Target target;
  request::Version version;
  request::Headers headers;
  Address client_address;
  std::string raw_body;
};
std::ostream &operator<<(std::ostream &o, Request &rq);
