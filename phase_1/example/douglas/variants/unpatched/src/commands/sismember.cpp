#include <iostream>
#include <set>
#include <string>

#include "../assert.h"
#include "../log.h"
#include "../set.h"

#include "sismember.h"

using std::string;

void Sismember::respond() {
  lll("processing sismember\n");

  std::vector<RespEntry> args = argv.get_array();
  args.erase(args.begin());

  string key = args[0].get_string();
  string candidate = args[1].get_string();

  Set s = Set{key};

  if (s.member(candidate)) {
    RespEntry{1l}.dump(out);
  } else {
    RespEntry{0l}.dump(out);
  }
}

bool Sismember::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "SISMEMBER") return false;
  if (cmd.get_array().size() != 3) return false;

  return true;
}
