#include <iostream>
#include <string>

#include "../assert.h"
#include "../db.h"
#include "../log.h"
#include "../resp.h"

#include "get.h"

using std::string;

void Get::respond() {
  lll("Processing GET\n");
  assert(RespEntry::array == argv.kind);

  RespEntry key_holder = argv.arr_val[1];
  assert(RespEntry::string == key_holder.kind);
  string key = key_holder.string_val;

  string value = Db::get_instance()->read(key);

  RespEntry{value}.dump(out);
}

bool Get::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "GET") return false;
  if (cmd.get_array().size() != 2) return false;

  return true;
}
