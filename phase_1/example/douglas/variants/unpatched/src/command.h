#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "resp.h"

class Command {
 public:
  Command(std::ostream& o, RespEntry a) : argv(a), out(o){};

  virtual void respond();

  static bool handles(RespEntry cmd);

  static Command* command_for(std::ostream& out, RespEntry& cmd);

 protected:
  RespEntry argv;
  std::ostream& out;
};
