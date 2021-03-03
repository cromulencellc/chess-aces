#pragma once

#include "../command.hpp"

namespace command {
  class Fetch : public Base {
  public:
    Fetch() {};

    virtual bool execute(Io& io, std::string args) override;
  };
}
