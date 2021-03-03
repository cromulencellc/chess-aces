#pragma once

#include <iostream>
#include <string>

#include "../command.h"
#include "../resp.h"

class Scard : public Command {
 public:
  Scard(std::ostream& o, RespEntry a) : Command(o, a){};

  virtual void respond();

  static bool handles(RespEntry cmd);
};
