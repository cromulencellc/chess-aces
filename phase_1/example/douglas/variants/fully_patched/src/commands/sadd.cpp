#include <iostream>
#include <set>
#include <string>

#include "../assert.h"
#include "../log.h"
#include "../set.h"

#include "sadd.h"

using std::string;

void Sadd::respond() {
  lll("processing sadd\n");

  std::vector<RespEntry> args = argv.get_array();
  args.erase(args.begin());

  string key = args[0].get_string();
  args.erase(args.begin());

  Set s = Set{key};

  long int new_entry_count = 0;

  while (args.size() >= 1) {
    string candidate = args[0].get_string();
    if (!s.member(candidate)) {
      new_entry_count++;
      s.add(candidate);
    }

    args.erase(args.begin());
  }

  s.write();

  RespEntry{new_entry_count}.dump(out);
}

bool Sadd::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "SADD") return false;
  if (cmd.get_array().size() < 2) return false;

  return true;
}
