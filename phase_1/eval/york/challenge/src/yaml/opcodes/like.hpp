#pragma once

#include "../opcode.hpp"

namespace yaml::opcodes {
  class Like : public Opcode {
  public:
    Like(std::string_view& query_str);
    virtual ~Like() {};

    void exec(Stack& stk, const ColumnCollection& _cols, const Row& _row) override;
  };
}
