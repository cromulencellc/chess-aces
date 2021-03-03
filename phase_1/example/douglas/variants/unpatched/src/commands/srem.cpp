#include <iostream>
#include <set>
#include <string>

#include "../assert.h"
#include "../log.h"
#include "../set.h"

#include "srem.h"

using std::string;

void Srem::respond() {
  lll("processing srem\n");

  std::vector<RespEntry> args = argv.get_array();
  args.erase(args.begin());

  string key = args[0].get_string();
  args.erase(args.begin());

  Set s = Set{key};

  long int found_entry_count = 0;

  while (args.size() >= 1) {
    string candidate = args[0].get_string();
    if (s.member(candidate)) {
      found_entry_count++;
      s.remove(candidate);
    }

    args.erase(args.begin());
  }

  s.write();

  RespEntry{found_entry_count}.dump(out);
}

bool Srem::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "SREM") return false;
  if (cmd.get_array().size() < 2) return false;

  return true;
}
