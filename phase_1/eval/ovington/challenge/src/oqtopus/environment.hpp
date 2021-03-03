#pragma once

#include "cell.hpp"
#include "native_lambda.hpp"
#include "value.hpp"

#include <map>
#include <string>
#include <variant>

namespace oqtopus {
  using _Environment_Base = std::map<std::string, ValuePtr>;

  class Environment {
  public:
    Environment(); // default environment
    Environment(_Environment_Base i) : mine(i) {};

    ValuePtr operator[](std::string k);
    void set(std::string k, ValuePtr v);

    void reparent(Environment& e);

    std::string inspect() const;

  private:
    _Environment_Base mine;
    std::shared_ptr<Environment> parent = nullptr;
  };

  Environment _make_default_environment();

  const Environment default_environment = _make_default_environment();
}

std::ostream& operator<<(std::ostream& os, const oqtopus::Environment& e);
