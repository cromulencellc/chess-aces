#include "method.hpp"

#include "error.hpp"

using namespace http;

Method::Method(std::string_view& request_line) : std::string(request_line) {
  std::string::size_type first_space = find_first_of(' ');

  if (0 == first_space) {
    throw BadRequest("couldn't parse method (leading space in request line");
  }
  if (std::string_view::npos == first_space) {
    throw BadRequest("couldn't parse method (no space in request line");
  }

  request_line.remove_prefix(first_space + 1);

  resize(first_space);

  if (0 == compare("GET")) {
    name = Name::GET;
    return;
  }
  if (0 == compare("HEAD")) {
    name = Name::HEAD;
    return;
  }
  if (0 == compare("POST")) {
    name = Name::POST;
    return;
  }
  if (0 == compare("PUT")) {
    name = Name::PUT;
    return;
  }
  if (0 == compare("DELETE")) {
    name = Name::DELETE;
    return;
  }
  if (0 == compare("CONNECT")) {
    name = Name::CONNECT;
    return;
  }
  if (0 == compare("OPTIONS")) {
    name = Name::OPTIONS;
    return;
  }
  if (0 == compare("TRACE")) {
    name = Name::TRACE;
    return;
  }
  name = Name::_INVALID;
}

bool Method::is_valid() const {
  if (Name::_INVALID == name) return false;

  return true;
}
