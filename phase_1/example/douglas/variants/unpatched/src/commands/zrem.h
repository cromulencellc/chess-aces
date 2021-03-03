#pragma once

#include <iostream>
#include <string>

#include "../command.h"
#include "../resp.h"

class Zrem : public Command {
 public:
  Zrem(std::ostream& o, RespEntry a) : Command(o, a){};

  virtual void respond();

  static bool handles(RespEntry cmd);
};
