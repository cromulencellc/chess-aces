#pragma once

#include <iostream>
#include <string>

#include "../command.h"
#include "../resp.h"

class Zrange : public Command {
 public:
  Zrange(std::ostream& o, RespEntry a) : Command(o, a){};

  virtual void respond();

  static bool handles(RespEntry cmd);
};
