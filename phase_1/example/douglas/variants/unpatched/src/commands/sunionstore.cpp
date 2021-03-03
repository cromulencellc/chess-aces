#include <iostream>
#include <set>
#include <string>

#include "../assert.h"
#include "../log.h"
#include "../set.h"

#include "sunionstore.h"

using std::string;

void Sunionstore::respond() {
  lll("processing sunion\n");

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
    std::vector<string> ov = other.to_vec();
    for (string e : ov) {
      if (!main_set.member(e)) main_set.add(e);
    }
  }

  main_set.write(dest_key);

  RespEntry{main_set.count()}.dump(out);
}

bool Sunionstore::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "SUNIONSTORE") return false;
  if (cmd.get_array().size() < 3) return false;

  return true;
}
