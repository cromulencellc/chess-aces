#pragma once

#include <filesystem>
#include <string>

#include "assert.h"
#include "resp.h"

using Path = std::filesystem::path;

class Db {
 public:
  static Db* get_instance();

  void dump_keys(std::ostream& o);
  std::string read(std::string key_name);
  void write(std::string key, std::string value);
  void append(std::string key, std::string value);
  long int length(std::string key);

  RespEntry deserialize(std::string key);
  void serialize(std::string key, RespEntry value);

 private:
  Db();

  Path db_path;
};
