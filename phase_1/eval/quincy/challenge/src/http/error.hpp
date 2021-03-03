#pragma once

#include "../error.hpp"

namespace http {
  class SystemError : public ::SystemError {};
  class RuntimeError : public ::RuntimeError {
  public:
    RuntimeError(std::string why) : ::RuntimeError(why) {}
  };
  class AddrInfoError : public http::RuntimeError {
  public:
    AddrInfoError(int code);
  };
}
