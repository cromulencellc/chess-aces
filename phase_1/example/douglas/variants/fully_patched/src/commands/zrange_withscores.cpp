#include <iostream>
#include <string>

#include "../log.h"
#include "../zset.h"

#include "zrange_withscores.h"

using std::string;

void ZrangeWithscores::respond() {
  lll("processing zrange withscores\n");

  string key = argv.get_array()[1].get_string();

  long int start = argv.get_array()[2].cast_integer();
  long int stop = argv.get_array()[3].cast_integer();

  Zset s = Zset{key};

  std::vector<ZScorePair> pairs = s.range(start, stop);

  std::vector<RespEntry> entries = {};
  entries.reserve(pairs.size());

  for (ZScorePair p : pairs) {
    entries.push_back(p.second);
    entries.push_back(p.first);
  }

  RespEntry{entries}.dump(out);
}

bool ZrangeWithscores::handles(RespEntry cmd) {
  if (!(cmd.first_string_upcase() == "ZRANGE")) return false;
  if (!(cmd.get_array().size() == 5)) return false;
  if (!(cmd.get_array()[4].get_string() == "WITHSCORES")) return false;

  return true;
}
