#include <iostream>
#include <string>

#include "../log.h"
#include "../zset.h"

#include "zrange.h"

using std::string;

void Zrange::respond() {
  lll("processing zrange\n");

  string key = argv.get_array()[1].get_string();

  long int start = argv.get_array()[2].cast_integer();
  long int stop = argv.get_array()[3].cast_integer();

  Zset s = Zset{key};

  std::vector<ZScorePair> pairs = s.range(start, stop);

  std::vector<RespEntry> entries = {};
  entries.reserve(pairs.size());

  for (ZScorePair p : pairs) {
    entries.push_back(p.second);
  }

  RespEntry{entries}.dump(out);
}

bool Zrange::handles(RespEntry cmd) {
  if (!(cmd.first_string_upcase() == "ZRANGE")) return false;
  if (cmd.get_array().size() != 4) return false;

  return true;
}
