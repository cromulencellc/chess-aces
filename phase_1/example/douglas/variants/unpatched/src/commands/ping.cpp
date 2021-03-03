#include <iostream>
#include <string>

#include "../log.h"

#include "ping.h"

using std::string;

void Ping::respond() {
  lll("Processing %s\n", argv.first_string().c_str());
  RespEntry{"PONG"}.dump(out);
}

bool Ping::handles(RespEntry& cmd) {
  if (cmd.first_string_upcase() != "PING") return false;
  if (cmd.get_array().size() != 1) return false;

  return true;
}
