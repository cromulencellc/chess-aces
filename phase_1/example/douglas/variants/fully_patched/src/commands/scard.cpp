#include <iostream>
#include <set>
#include <string>

#include "../assert.h"
#include "../log.h"
#include "../set.h"

#include "scard.h"

using std::string;

void Scard::respond() {
  lll("processing scard\n");

  std::vector<RespEntry> args = argv.get_array();
  args.erase(args.begin());

  string key = args[0].get_string();

  Set s = Set{key};

  RespEntry{s.count()}.dump(out);
}

bool Scard::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "SCARD") return false;
  if (cmd.get_array().size() != 2) return false;

  return true;
}
