#pragma once
#include <errno.h>
#include <stdexcept>
#include <system_error>
class MontagueError {};
class MontagueSystemError : public MontagueError, public std::system_error {
public:
  MontagueSystemError()
      : MontagueError(), std::system_error{errno, std::system_category()} {};
};
class MontagueRuntimeError : public MontagueError, public std::runtime_error {
public:
  MontagueRuntimeError()
      : MontagueError(), std::runtime_error("Montague Runtime Error"){};
  MontagueRuntimeError(std::string explanation)
      : MontagueError(), std::runtime_error{explanation} {};
};
