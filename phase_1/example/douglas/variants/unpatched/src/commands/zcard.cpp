#include <iostream>
#include <string>

#include "../log.h"
#include "../zset.h"

#include "zcard.h"

using std::string;

void Zcard::respond() {
  lll("processing zcard\n");

  string key = argv.get_array()[1].get_string();

  Zset s = Zset{key};

  RespEntry{s.count()}.dump(out);
}

bool Zcard::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "ZCARD") return false;
  if (cmd.get_array().size() != 2) return false;

  return true;
}
