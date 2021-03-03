#pragma once

#include <string>
#include <string_view>

namespace http {
  class Method : public std::string {
  public:
    Method(std::string_view& request_line);

    Method() : std::string(""), name(Name::_INVALID) {};

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

    Name name = Name::_INVALID;

    bool is_valid() const;
  };
}
