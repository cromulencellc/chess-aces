#pragma once

#include <optional>
#include <string>

#include "../error.hpp"

namespace http {
  class Url {
  public:
    Url() : valid(false) {};
    Url(std::string url);

    std::string scheme;
    std::string host;
    std::string port;
    std::string target;

    bool valid;

    std::string inspect() const;

    uint16_t port_number(uint16_t def = 80);

  private:
    std::optional<uint16_t> _port_number = {};
  };

  class InvalidUrlError : public RuntimeError {
  public:
    InvalidUrlError(std::string not_url) :
      RuntimeError("couldn't parse invalid url `" + not_url + "`") {}
  };

  class InvalidPortNumberError : public RuntimeError {
  public:
    InvalidPortNumberError(std::string not_port) :
      RuntimeError("couldn't find a port number in `" +
                   not_port + "`") {}
  };
}

std::ostream& operator<<(std::ostream& o, const http::Url& u);
