#include <iostream>
#include <set>
#include <string>

#include "../assert.h"
#include "../log.h"
#include "../set.h"

#include "sinter.h"

using std::string;

void Sinter::respond() {
  lll("processing sinter\n");

  std::vector<RespEntry> args = argv.get_array();
  args.erase(args.begin());

  string main_key = args[0].get_string();
  args.erase(args.begin());

  Set main_set = Set{main_key};
  std::vector<string> main_members = main_set.to_vec();

  while (1 <= args.size()) {
    Set other = Set{args[0].get_string()};
    args.erase(args.begin());
    for (string e : main_members) {
      if (!other.member(e)) main_set.remove(e);
    }
  }

  RespEntry{main_set.to_resp()}.dump(out);
}

bool Sinter::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "SINTER") return false;
  if (cmd.get_array().size() < 2) return false;

  return true;
}
