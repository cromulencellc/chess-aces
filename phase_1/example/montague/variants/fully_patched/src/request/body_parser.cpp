#include "body_parser.hpp"
#include "../error.hpp"
#include <charconv>
using namespace request;
using namespace request::body_parser;
FormEncoded::FormEncoded(std::string raw_body) {
  std::vector<std::string> pairs = split_pairs(raw_body);
  std::vector<std::pair<std::string, std::string>> kvs = {};
  for (auto pair : pairs) {
    kvs.push_back(split_kv(pair));
  }
  form_data = {};
  for (auto pair : kvs) {
    form_data.emplace(pair);
  }
}
std::vector<std::string> FormEncoded::split_pairs(std::string line) {
  std::vector<std::string> got = {};
  if (0 == line.size())
    return got;
  size_t cursor = 0;
  while (true) {
    size_t found = line.find("&", cursor);
    if (std::string::npos == found)
      break;
    got.push_back(line.substr(cursor, (found - cursor)));
    cursor = found;
  }
  got.push_back(line.substr(cursor, (line.size() - cursor)));
  return got;
}
std::pair<std::string, std::string> FormEncoded::split_kv(std::string pair) {
  std::string key;
  std::string value;
  size_t got = pair.find("=");
  if (std::string::npos == got) {
    throw MontagueRuntimeError("couldn't parse key value pair " + pair);
  }
  key = pair.substr(0, got);
  value = pair.substr(got + 1, pair.size() - (got + 1));
  return {decode(key), decode(value)};
}
std::string FormEncoded::decode(std::string unit) {
  size_t cursor = 0;
  while (true) {
    size_t found = unit.find("+", cursor);
    if (std::string::npos == found)
      break;
    unit.replace(found, 1, " ");
    cursor = found;
  }
  cursor = 0;
  while (true) {
    size_t found = unit.find("%", cursor);
    if (std::string::npos == found)
      break;
    char *hexits_start = unit.data() + found + 1;
    char *hexits_end = unit.data() + found + 2;
    char replacement;
    auto from_got = std::from_chars(hexits_start, hexits_end, replacement, 16);
    if (hexits_end != from_got.ptr) {
      throw MontagueRuntimeError("couldn't parse percent " + unit);
    }
    cursor = found + 3;
  }
  return unit;
}
