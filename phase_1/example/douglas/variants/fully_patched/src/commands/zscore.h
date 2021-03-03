#pragma once

#include <iostream>
#include <string>

#include "../command.h"
#include "../resp.h"

class Zscore : public Command {
 public:
  Zscore(std::ostream& o, RespEntry a) : Command(o, a){};

  virtual void respond();

  static bool handles(RespEntry cmd);
};
