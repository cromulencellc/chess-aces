#include "program.hpp"

#include "stack.hpp"

#include "../logger.hpp"

using namespace yaml;

Program::Program(const std::string& query_str) {
  std::string_view running = {query_str};

  while (running.size() > 0) {
    codes.push_back(Opcode::next_opcode(running));
  }
}

bool Program::matches(const ColumnCollection& cols, const Row& row) const {
  Stack stk;

  for (OpcodePtr opp : codes) {
    opp->exec(stk, cols, row);
  }

  LLL.debug() << "stack size " << stk.size()
              << " top is bool?(" << stk.top_is_bool()
              << ") string?(" << stk.top_is_string()
              << ")";

  stk.assert_not_empty();
  bool got = stk.pop_bool();
  LLL.debug() << "matched? " << got;
  return got;
}
