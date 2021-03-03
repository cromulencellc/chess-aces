#include <iostream>
#include <set>
#include <string>

#include "../assert.h"
#include "../log.h"
#include "../set.h"

#include "sunion.h"

using std::string;

void Sunion::respond() {
  lll("processing sunion\n");

  std::vector<RespEntry> args = argv.get_array();
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

  RespEntry{main_set.to_resp()}.dump(out);
}

bool Sunion::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "SUNION") return false;
  if (cmd.get_array().size() < 2) return false;

  return true;
}
