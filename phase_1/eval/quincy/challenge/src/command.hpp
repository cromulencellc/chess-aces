#pragma once

#include "error.hpp"
#include "io.hpp"
#include "logger.hpp"

#include <memory>
#include <string>

namespace command {
  class Base {
  public:
    virtual ~Base() {};

    static std::unique_ptr<Base> match(std::string name);

    virtual bool execute(Io& io, std::string args) = 0;
  };

}

using Command = command::Base;
