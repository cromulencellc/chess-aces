#pragma once

#include <iostream>
#include <string>

#include "../command.h"
#include "../resp.h"

class Set : public Command {
 public:
  Set(std::ostream& o, RespEntry a) : Command(o, a){};

  virtual void respond();

  static bool handles(RespEntry cmd);
};
