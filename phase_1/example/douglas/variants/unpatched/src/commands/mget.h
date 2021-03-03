#pragma once

#include <iostream>
#include <string>

#include "../command.h"
#include "../resp.h"

class Mget : public Command {
 public:
  Mget(std::ostream& o, RespEntry a) : Command(o, a){};

  virtual void respond();

  static bool handles(RespEntry cmd);
};
