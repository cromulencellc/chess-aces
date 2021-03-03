#pragma once

#include "cell.hpp"
#include "environment.hpp"
#include "value.hpp"

namespace oqtopus {
  class RowLambda : public value::Base {
  public:
    RowLambda(ValuePtr b, Environment base) : body(b), base_env(base) {}
    virtual ~RowLambda() {};

    virtual std::string inspect() const;

    ValuePtr call(Environment overlay);
  private:
    ValuePtr body;
    Environment base_env;
  };
}
