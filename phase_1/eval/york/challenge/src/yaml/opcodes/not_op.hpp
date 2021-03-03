#pragma once

#include "../opcode.hpp"

namespace yaml::opcodes {
  class NotOp : public Opcode {
  public:
    NotOp(std::string_view& query_str);
    virtual ~NotOp() {};

    void exec(Stack& stk, const ColumnCollection& _cols, const Row& _row) override;
  };
}
