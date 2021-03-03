#pragma once

#include <filesystem>
#include <string>

#include "assert.h"
#include "resp.h"

class Path : public std::filesystem::path {
 public:
  Path(std::string p, bool s);
  Path(const Path& o);

  bool safe();

 private:
  bool sticky = false;
  std::filesystem::path parent = "";

  friend Path operator/(const Path& lhs, const std::filesystem::path& rhs);
};


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
