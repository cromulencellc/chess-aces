#include "error.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

using namespace http;

AddrInfoError::AddrInfoError(int code) :
  http::RuntimeError("getaddrinfo failed with code " +
                     std::to_string(code) +
                     ": " +
                     std::string(gai_strerror(code)))
{}
