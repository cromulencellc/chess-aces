#include <algorithm>
#include <iostream>
#include <string>

#include "../log.h"
#include "../zset.h"

#include "zrevrange.h"

using std::string;

void Zrevrange::respond() {
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

  std::reverse(entries.begin(), entries.end());

  RespEntry{entries}.dump(out);
}

bool Zrevrange::handles(RespEntry cmd) {
  if (!(cmd.first_string_upcase() == "ZREVRANGE")) return false;
  if (cmd.get_array().size() != 4) return false;

  return true;
}
