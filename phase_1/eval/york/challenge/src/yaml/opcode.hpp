#pragma once

#include <memory>
#include <string_view>

#include "column_collection.hpp"
#include "error.hpp"
#include "row.hpp"
#include "stack.hpp"

namespace yaml {
  class Opcode;

  using OpcodePtr = std::shared_ptr<Opcode>;

  class Opcode {
  public:
    static OpcodePtr next_opcode(std::string_view& query_str);
    virtual ~Opcode() {};
    virtual void exec(Stack& stk,
                      const ColumnCollection& cols,
                      const Row& row) = 0;

    enum class Code : char {
                            push_str = '"',
                            deref = '*',
                            eq = '=',
                            like = '~',
                            not_op = '!',
                            and_op = '&',
                            or_op = '|',
                            xor_op = '^',
                            push_true = 't',
                            push_false = 'f'
    };
  };
}
