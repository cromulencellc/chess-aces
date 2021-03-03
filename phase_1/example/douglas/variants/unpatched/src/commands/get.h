#pragma once

#include <iostream>
#include <string>

#include "../command.h"

class Get : public Command {
 public:
  Get(std::ostream& o, RespEntry a) : Command(o, a){};

  virtual void respond();

  static bool handles(RespEntry cmd);
};
