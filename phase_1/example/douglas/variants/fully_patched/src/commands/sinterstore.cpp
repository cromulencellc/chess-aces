#include <iostream>
#include <set>
#include <string>

#include "../assert.h"
#include "../log.h"
#include "../set.h"

#include "sinterstore.h"

using std::string;

void Sinterstore::respond() {
  lll("processing sinterstore\n");

  std::vector<RespEntry> args = argv.get_array();
  args.erase(args.begin());

  string dest_key = args[0].get_string();
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

  main_set.write(dest_key);

  RespEntry{main_set.count()}.dump(out);
}

bool Sinterstore::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "SINTERSTORE") return false;
  if (cmd.get_array().size() < 3) return false;

  return true;
}
