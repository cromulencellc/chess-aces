#include <iostream>
#include <set>
#include <string>

#include "../assert.h"
#include "../log.h"
#include "../set.h"

#include "smembers.h"

using std::string;

void Smembers::respond() {
  lll("processing smembers\n");

  std::vector<RespEntry> args = argv.get_array();
  args.erase(args.begin());

  string key = args[0].get_string();

  Set s = Set{key};

  RespEntry{s.to_resp()}.dump(out);
}

bool Smembers::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "SMEMBERS") return false;
  if (cmd.get_array().size() != 2) return false;

  return true;
}
