#pragma once

#include "../opcode.hpp"

namespace yaml::opcodes {
  class Eq : public Opcode {
  public:
    Eq(std::string_view& query_str);
    virtual ~Eq() {};

    void exec(Stack& stk, const ColumnCollection& _cols, const Row& _row) override;
  };
}
