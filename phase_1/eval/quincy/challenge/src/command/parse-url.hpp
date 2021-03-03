#pragma once

#include "../command.hpp"

namespace command {
  class ParseUrl : public Base {
  public:
    ParseUrl() {};

    virtual bool execute(Io& io, std::string args) override;
  };
}
