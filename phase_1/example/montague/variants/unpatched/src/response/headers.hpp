#pragma once
#include <map>
#include <ostream>
#include <string>
#include "header.hpp"
namespace response {
using Headers = std::map<std::string, std::string>;
}
std::ostream &operator<<(std::ostream &o, response::Headers &hs);
