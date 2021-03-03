#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "headers.hpp"
namespace request {
namespace body_parser {
using DecodedForm = std::multimap<std::string, std::string>;
class FormEncoded {
public:
  FormEncoded(std::string raw_body);
  DecodedForm form_data;
private:
  std::string decode(std::string unit);
  std::vector<std::string> split_pairs(std::string line);
  std::pair<std::string, std::string> split_kv(std::string pair);
  std::string _original;
};
}
}
