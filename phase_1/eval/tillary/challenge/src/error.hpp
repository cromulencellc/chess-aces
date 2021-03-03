#pragma once

#include <errno.h>
#include <stdexcept>
#include <string>
#include <system_error>

#ifdef BACKTRACES
#include <array>

#ifndef BACKTRACE_LIMIT
#define BACKTRACE_LIMIT 32
#endif
#endif

#define error_string_constructor(error_class, error_superclass) \
  error_class(std::string explanation) : error_superclass(explanation) \
  {}

class Error {
public:
#ifdef BACKTRACES
  std::array<void*, BACKTRACE_LIMIT> frames;
  size_t valid_frames = 0;

  Error();
#endif

  virtual std::string inspect() const;
};

class SystemError : public Error,
                    public std::system_error {
public:
  SystemError() : Error(),
                  std::system_error{errno, std::system_category()} {};

  virtual std::string inspect() const override;
};

class RuntimeError : public Error,
                     public std::runtime_error {
public:
  RuntimeError(std::string explanation) : Error(),
                                          std::runtime_error(explanation) {};

  RuntimeError() : RuntimeError("Runtime Error") {};

  virtual std::string inspect() const override;
};
