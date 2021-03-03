#pragma once

#include "../opcode.hpp"

namespace yaml::opcodes {
  class OrOp : public Opcode {
  public:
    OrOp(std::string_view& query_str);
    virtual ~OrOp() {};

    void exec(Stack& stk, const ColumnCollection& _cols, const Row& _row) override;
  };
}
