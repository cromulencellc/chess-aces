#pragma once

#include <iostream>
#include <string>

#include "../command.h"

class Ping : public Command {
 public:
  Ping(std::ostream& o, RespEntry a) : Command(o, a){};

  virtual void respond();

  static bool handles(RespEntry& cmd);
};
