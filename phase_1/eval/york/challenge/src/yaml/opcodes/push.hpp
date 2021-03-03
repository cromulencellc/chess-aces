#pragma once

#include "../opcode.hpp"

namespace yaml::opcodes {
  class PushTrue : public Opcode {
  public:
    PushTrue(std::string_view& query_str);
    virtual ~PushTrue() {};

    void exec(Stack& stk,
              const ColumnCollection& _cols,
              const Row& _row) override;

    virtual bool to_push() { return true; }
  };

  class PushFalse : public PushTrue {
  public:
    PushFalse(std::string_view& query_str) : PushTrue(query_str) {};
    virtual ~PushFalse() {};

    bool to_push() override { return false; }
  };
}
