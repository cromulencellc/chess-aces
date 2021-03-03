#pragma once

#include <iostream>
#include <string>

#include "../command.h"
#include "../resp.h"

class Zcard : public Command {
 public:
  Zcard(std::ostream& o, RespEntry a) : Command(o, a){};

  virtual void respond();

  static bool handles(RespEntry cmd);
};
