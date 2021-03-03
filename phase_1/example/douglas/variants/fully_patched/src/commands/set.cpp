#include <iostream>
#include <string>

#include "../assert.h"
#include "../db.h"
#include "../log.h"

#include "set.h"

using std::string;

void Set::respond() {
  lll("Processing SET\n");

  assert(RespEntry::array == argv.kind);
  RespEntry keyholder = argv.arr_val[1];
  RespEntry valueholder = argv.arr_val[2];

  assert(RespEntry::string == keyholder.kind);
  assert(RespEntry::string == valueholder.kind);

  string key = keyholder.string_val;
  string value = valueholder.string_val;

  Db::get_instance()->write(key, value);

  RespEntry{"OK"}.dump(out);
}

bool Set::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "SET") return false;
  if (cmd.get_array().size() < 3) return false;

  return true;
}
