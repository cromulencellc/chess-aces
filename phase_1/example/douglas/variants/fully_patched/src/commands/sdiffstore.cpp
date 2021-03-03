#include <iostream>
#include <set>
#include <string>

#include "../assert.h"
#include "../log.h"
#include "../set.h"

#include "sdiffstore.h"

using std::string;

void Sdiffstore::respond() {
  lll("processing sdiffstore\n");

  std::vector<RespEntry> args = argv.get_array();
  args.erase(args.begin());

  string dest_key = args[0].get_string();
  args.erase(args.begin());

  string main_key = args[0].get_string();
  args.erase(args.begin());

  Set main_set = Set{main_key};

  while (1 <= args.size()) {
    Set other = Set{args[0].get_string()};
    args.erase(args.begin());
    std::vector<string> om = other.to_vec();
    for (string e : om) {
      if (main_set.member(e)) main_set.remove(e);
    }
  }

  main_set.write(dest_key);

  RespEntry{main_set.count()}.dump(out);
}

bool Sdiffstore::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "SDIFFSTORE") return false;
  if (cmd.get_array().size() < 3) return false;

  return true;
}
