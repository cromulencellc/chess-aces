#include <iostream>
#include <string>

#include "../assert.h"
#include "../db.h"
#include "../log.h"

#include "strlen.h"

using std::string;

void Strlen::respond() {
  lll("processing STRLEN\n");

  assert(RespEntry::array == argv.kind);
  RespEntry keyholder = argv.arr_val[1];

  assert(RespEntry::string == keyholder.kind);

  string key = keyholder.string_val;

  long int len = Db::get_instance()->length(key);

  RespEntry{len}.dump(out);
}

bool Strlen::handles(RespEntry cmd) {
  if (cmd.first_string_upcase() != "STRLEN") return false;
  if (cmd.get_array().size() != 2) return false;

  return true;
}
