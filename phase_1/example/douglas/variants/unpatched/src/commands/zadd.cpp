#include <iostream>
#include <string>

#include "../assert.h"
#include "../log.h"
#include "../zset.h"

#include "zadd.h"

using std::string;

void Zadd::respond() {
  lll("processing zadd\n");

  std::vector<RespEntry> args = argv.get_array();
  args.erase(args.begin());

  string key = args[0].get_string();
  args.erase(args.begin());

  assert(0 == (args.size() % 2));

  Zset s = Zset(key);

  long int pre_count = s.count();

  while (args.size() >= 2) {
    double score = args[0].cast_double();
    string candidate = args[1].get_string();
    args.erase(args.begin());
    args.erase(args.begin());

    s.add(score, candidate);
  }

  long int post_count = s.count();

  s.write();

  RespEntry{post_count - pre_count}.dump(out);
}

bool Zadd::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "ZADD") return false;
  if (cmd.get_array().size() < 2) return false;

  return true;
}
