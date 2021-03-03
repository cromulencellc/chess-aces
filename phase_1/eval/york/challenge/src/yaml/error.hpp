#pragma once

#include "../error.hpp"

namespace yaml {
  class RuntimeError : public ::RuntimeError {
  public:
    RuntimeError() : ::RuntimeError("YAML Runtime Error") {};
    error_string_constructor(RuntimeError, ::RuntimeError);
  };

  class SyntaxError : public yaml::RuntimeError {
  public:
    SyntaxError() : yaml::RuntimeError("YAML query syntax error") {};
    error_string_constructor(SyntaxError, yaml::RuntimeError);
  };

  class TypeError : public yaml::RuntimeError {
  public:
    TypeError() : yaml::RuntimeError("YAML type error") {};
    error_string_constructor(TypeError, yaml::RuntimeError);
  };

  class EmptyStackError : public yaml::RuntimeError {
  public:
    EmptyStackError() : yaml::RuntimeError("YAML stack unexpectedly empty") {};
  };

  class UnknownColumnError : public yaml::RuntimeError {
  public:
    UnknownColumnError(std::string cn) :
      yaml::RuntimeError("YAML unknown column " + cn) {};
  };

  class UnknownOperationError : public yaml::RuntimeError {
  public:
    UnknownOperationError(int op) :
      yaml::RuntimeError(std::string("YAML unexpected opcode ") +
                         std::to_string(op)) {};
  };

  class ImplementationError : public yaml::RuntimeError {
  public:
    ImplementationError() :
      yaml::RuntimeError("YAML implementation error D:") {};
    error_string_constructor(ImplementationError, yaml::RuntimeError);
  };
}
