#include <iostream>
#include <string>
#include <vector>

#include "../db.h"
#include "../log.h"

#include "mget.h"

using std::string;

void Mget::respond() {
  lll("processing MGET\n");

  std::vector<RespEntry> args = argv.get_array();
  args.erase(args.begin());

  std::vector<RespEntry> result{};
  result.reserve(args.size());

  while (args.size() >= 1) {
    string key = args[0].get_string();
    string value = Db::get_instance()->read(key);

    result.push_back(RespEntry{value});
    args.erase(args.begin());
  }
  RespEntry{result}.dump(out);
}

bool Mget::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "MGET") return false;
  if (cmd.get_array().size() < 1) return false;

  return true;
}
