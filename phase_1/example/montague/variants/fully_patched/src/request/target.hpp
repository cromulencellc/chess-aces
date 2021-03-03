#pragma once
#include <filesystem>
#include <ostream>
#include <string>
#include "reader.hpp"
namespace request {
class Target {
public:
  static constexpr std::size_t max_path_len = 1 << 12;
  static constexpr std::size_t max_query_len = 1 << 12;
  Target(){};
  Target(std::string p, std::string q)
      : path(p, std::filesystem::path::format::generic_format), query(q){};
  Target(Reader &in);
  bool valid();
  friend std::ostream &operator<<(std::ostream &o, const Target &targ) {
    o << "Target(" << targ.path;
    if (targ._has_query) {
      o << "?" << targ.query;
    }
    o << ")";
    return o;
  }
  std::filesystem::path path;
private:
  bool _valid;
  bool _has_query;
  std::string query;
};
}
std::string to_string(const request::Target &targ);
