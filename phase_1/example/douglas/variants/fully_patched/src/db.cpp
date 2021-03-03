#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "assert.h"
#include "db.h"
#include "log.h"

#define DATABASE_DIR "/data"s

using std::string;

using namespace std::string_literals;

namespace fs = std::filesystem;

Path::Path(std::string p, bool s) : std::filesystem::path(p) {
  if (s) {
    sticky = true;
    parent = {p};
  };
}

Path::Path(const Path& o)
    : std::filesystem::path((std::filesystem::path)o),
      sticky(o.sticky),
      parent(o.parent){};

bool Path::safe() {
  if (!sticky) return false;
  std::string normalized = {lexically_normal()};
  std::string chunk = normalized.substr(0, ((std::string)parent).size());
  return chunk == parent;
}

Path operator/(const Path& lhs, const std::filesystem::path& rhs) {
  if (lhs.sticky) {
    Path copy = lhs;
    copy /= rhs;
    assert(copy.safe());
    return copy;
  }

  std::filesystem::path unstuck = std::filesystem::path{(std::string)lhs};
  unstuck /= rhs;
  return Path{unstuck, false};
}

Db::Db()
    :
      db_path(DATABASE_DIR, true)
{
  if (!fs::is_directory(db_path)) {
    lll("db_path %s isn't a directory\n", db_path.c_str());
    exit(-1);
  }
}

void Db::dump_keys(std::ostream& o) {
  for (const fs::directory_entry& e : fs::directory_iterator(db_path)) {
    o << e.path().filename().c_str() << std::endl;
  }
}

std::string Db::read(std::string key_name) {
  Path key_path = db_path / key_name;

  std::ifstream reader(key_path, std::ios::binary | std::ios::ate);
  auto size = reader.tellg();
  reader.seekg(0);

  string contents(size, '\0');

  reader.read(&contents[0], size);
  reader.close();

  return contents;
}

void Db::write(std::string key, std::string value) {
  Path key_path = db_path / key;
  std::ofstream writer(key_path, std::ios::binary | std::ios::trunc);
  writer << value;
  writer.close();
}

void Db::append(std::string key, std::string value) {
  Path key_path = db_path / key;
  std::ofstream writer(key_path, std::ios::binary | std::ios::app);
  writer << value;
  writer.close();
}

long int Db::length(std::string key) {
  Path key_path = db_path / key;
  std::ifstream counter(key_path, std::ios::binary | std::ios::ate);
  long int len = counter.tellg();
  counter.close();
  return len;
}

RespEntry Db::deserialize(std::string key) {
  Path key_path = db_path / key;
  std::ifstream reader(key_path, std::ios::binary | std::ios::ate);
  long int len = reader.tellg();
  if (1 > len) {
    reader.close();
    return RespEntry{};
  }
  reader.seekg(0);
  RespEntry e(reader);
  reader.close();
  return e;
}

void Db::serialize(std::string key, RespEntry value) {
  Path key_path = db_path / key;
  std::ofstream writer(key_path, std::ios::binary | std::ios::trunc);

  value.dump(writer);
  writer.close();
}

Db* Db::get_instance() {
  static Db instance;
  return &instance;
}
