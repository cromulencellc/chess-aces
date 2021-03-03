#pragma once

#include "../opcode.hpp"

namespace yaml::opcodes {
  class XorOp : public Opcode {
  public:
    XorOp(std::string_view& query_str);
    virtual ~XorOp() {};

    void exec(Stack& stk, const ColumnCollection& _cols, const Row& _row) override;
  };
}
