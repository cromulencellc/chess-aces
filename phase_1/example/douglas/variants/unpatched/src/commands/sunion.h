#pragma once

#include <iostream>
#include <string>

#include "../command.h"
#include "../resp.h"

class Sunion : public Command {
 public:
  Sunion(std::ostream& o, RespEntry a) : Command(o, a){};

  virtual void respond();

  static bool handles(RespEntry cmd);
};