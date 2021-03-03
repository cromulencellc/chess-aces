#pragma once

#include "../opcode.hpp"

namespace yaml::opcodes {
  class Deref : public Opcode {
  public:
    Deref(std::string_view& query_str);
    virtual ~Deref() {};

    void exec(Stack& stk, const ColumnCollection& cols, const Row& row) override;

  private:
    bool did_cache_column_number = false;
    std::string cached_column_name = "";
    size_t cached_column_number = false;

    void exec_without_cache(Stack& stk,
                            const ColumnCollection& cols,
                            const Row& row);

    void exec_without_cache(Stack& stk,
                            const ColumnCollection& cols,
                            const Row& row,
                            const std::string& column_name);

    size_t get_column_number(const ColumnCollection& cols,
                             const std::string& column_name);
  };
}
