#pragma once

#include "../opcode.hpp"

namespace yaml::opcodes {
  class AndOp : public Opcode {
  public:
    AndOp(std::string_view& query_str);
    virtual ~AndOp() {};

    void exec(Stack& stk, const ColumnCollection& _cols, const Row& _row) override;
  };
}
