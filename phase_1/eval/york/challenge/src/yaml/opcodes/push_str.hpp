#pragma once

#include "../opcode.hpp"

namespace yaml::opcodes {
  class PushStr : public Opcode {
  public:
    PushStr(std::string_view& query_str);
    virtual ~PushStr() {};

    void exec(Stack& stk, const ColumnCollection& cols, const Row& row) override;

  private:
    std::string immediate;
  };
}
