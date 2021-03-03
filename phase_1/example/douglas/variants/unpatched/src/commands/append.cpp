#include <iostream>
#include <string>

#include "../assert.h"
#include "../db.h"
#include "../log.h"

#include "append.h"

using std::string;

void Append::respond() {
  lll("processing APPEND\n");

  assert(RespEntry::array == argv.kind);
  RespEntry keyholder = argv.arr_val[1];
  RespEntry valueholder = argv.arr_val[2];

  assert(RespEntry::string == keyholder.kind);
  assert(RespEntry::string == valueholder.kind);

  string key = keyholder.string_val;
  string value = valueholder.string_val;

  Db::get_instance()->append(key, value);

  long int len = Db::get_instance()->length(key);

  RespEntry{len}.dump(out);
}

bool Append::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "APPEND") return false;
  if (cmd.get_array().size() != 3) return false;

  return true;
}
