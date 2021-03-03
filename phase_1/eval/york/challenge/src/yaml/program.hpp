#pragma once

#include "opcode.hpp"
#include "row.hpp"

#include <vector>

namespace yaml {
  class Program {
  public:
    Program(const std::string& query_str);

    bool matches(const ColumnCollection& cols, const Row& row) const;

  private:
    std::vector<OpcodePtr> codes;
  };
}
