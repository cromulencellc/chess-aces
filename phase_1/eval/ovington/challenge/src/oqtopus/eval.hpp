#pragma once

#include "cell.hpp"
#include "environment.hpp"
#include "value.hpp"

namespace oqtopus {
  ValuePtr eval(ValuePtr args, Environment& env);

  class EvalError : public OvingtonRuntimeError {
  public:
    EvalError() :
      OvingtonRuntimeError("eval error") {}

    EvalError(std::string what) :
      OvingtonRuntimeError("eval error: " + what) {}
  };
}
