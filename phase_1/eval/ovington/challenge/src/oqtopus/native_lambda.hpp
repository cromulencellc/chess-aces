#pragma once

#include "cell.hpp"

#include <functional>

namespace oqtopus {
  using _NativeLambdaFunction = std::function<ValuePtr(ValuePtr)>;
  class NativeLambda : public _NativeLambdaFunction,
                       public value::Base {
  public:
    NativeLambda(_NativeLambdaFunction fn) : _NativeLambdaFunction(fn) {};
    virtual ~NativeLambda() {};
#ifndef PATCH_DISABLE_NL_CAST_TO_FLOAT
    virtual float64_t cast_float() const;
#endif

    virtual std::string inspect() const;
  };

  namespace builtin {
    ValuePtr add(ValuePtr i);
    ValuePtr subtract(ValuePtr i);
    ValuePtr multiply(ValuePtr i);
    ValuePtr divide(ValuePtr i);
    ValuePtr car(ValuePtr i);
    ValuePtr cdr(ValuePtr i);

    ValuePtr lt(ValuePtr i);
    ValuePtr lteq(ValuePtr i);
    ValuePtr eq(ValuePtr i);
    ValuePtr gteq(ValuePtr i);
    ValuePtr gt(ValuePtr i);
    ValuePtr neq(ValuePtr i);

    ValuePtr inspect(ValuePtr i);
  }

  class ArgumentError : public OvingtonRuntimeError {
  public:
    ArgumentError() : OvingtonRuntimeError("argument error") {}

    ArgumentError(std::string what) :
      OvingtonRuntimeError("argument error: " + what) {}
  };

  class FunctionError : public OvingtonRuntimeError {
  public:
    FunctionError() : OvingtonRuntimeError("function error") {};

    FunctionError(std::string what) :
      OvingtonRuntimeError("function error: " + what) {}
  };
}
