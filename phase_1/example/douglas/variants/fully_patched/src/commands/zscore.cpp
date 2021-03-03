#include <iostream>
#include <string>

#include "../log.h"
#include "../zset.h"

#include "zscore.h"

using std::string;

void Zscore::respond() {
  lll("processing zscore\n");

  string key = argv.get_array()[1].get_string();
  string member = argv.get_array()[2].get_string();

  Zset s = Zset{key};

  if (!s.member(member)) {
    RespEntry{nullptr}.dump(out);
    return;
  }

  RespEntry{s.score(member)}.dump(out);
}

bool Zscore::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "ZSCORE") return false;
  if (cmd.get_array().size() != 3) return false;

  return true;
}
