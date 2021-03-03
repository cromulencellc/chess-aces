#pragma once

#include <iostream>
#include <string>

#include "../command.h"
#include "../resp.h"

class Sismember : public Command {
 public:
  Sismember(std::ostream& o, RespEntry a) : Command(o, a){};

  virtual void respond();

  static bool handles(RespEntry cmd);
};
