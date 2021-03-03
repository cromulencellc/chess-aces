#pragma once

#include "../command.hpp"

namespace command {
  class ParseDoc : public Base {
  public:
    ParseDoc() {};

    virtual bool execute(Io& io, std::string args) override;
  };
}
