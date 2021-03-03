#pragma once

#include <string>

class Rng {
public:
  static std::string get_printables(size_t count);
  static char get_printable();
};
