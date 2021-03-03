#include <iostream>
#include <string>

#include "../log.h"
#include "../zset.h"

#include "zrem.h"

using std::string;

void Zrem::respond() {
  lll("processing zrem\n");

  std::vector<RespEntry> args = argv.get_array();
  args.erase(args.begin());
  string key = args[0].get_string();
  args.erase(args.begin());

  Zset s = Zset{key};

  long int start_count = s.count();

  while (1 <= args.size()) {
    string member = args[0].get_string();
    args.erase(args.begin());
    s.remove(member);
  }

  long int end_count = s.count();

  s.write();

  RespEntry{start_count - end_count}.dump(out);
}

bool Zrem::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "ZREM") return false;
  if (cmd.get_array().size() < 2) return false;

  return true;
}
