#include <iostream>
#include <string>
#include <vector>

#include "../assert.h"
#include "../db.h"
#include "../log.h"

#include "mset.h"

using std::string;

void Mset::respond() {
  lll("processing MSET\n");

  assert(RespEntry::array == argv.kind);
  std::vector<RespEntry> args = argv.arr_val;

  args.erase(args.begin());  

  if (0 != (args.size() % 2)) {
    lll("can't mset with an odd number of args\n");
    std::exit(-1);
  }

  while (args.size() >= 2) {
    string key = args[0].get_string();
    string value = args[1].get_string();

    Db::get_instance()->write(key, value);

    args.erase(args.begin());
    args.erase(args.begin());
  }

  assert(args.size() == 0);

  RespEntry{"OK"}.dump(out);
}

bool Mset::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "MSET") return false;
  if (cmd.get_array().size() < 1) return false;

  return true;
}
