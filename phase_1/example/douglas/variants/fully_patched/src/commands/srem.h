#pragma once

#include <iostream>
#include <string>

#include "../command.h"
#include "../resp.h"

class Srem : public Command {
 public:
  Srem(std::ostream& o, RespEntry a) : Command(o, a){};

  virtual void respond();

  static bool handles(RespEntry cmd);
};
