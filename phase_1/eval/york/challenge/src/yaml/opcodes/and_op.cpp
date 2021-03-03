#include "and_op.hpp"

using namespace yaml;
using namespace yaml::opcodes;

AndOp::AndOp(std::string_view& query_str) {
  if (0 == query_str.size()) {
    throw ImplementationError("query string unexpectedly empty");
  }

  if ((char)Opcode::Code::and_op != query_str[0]) {
    throw ImplementationError("looked for and_op opcode, got " +
                              std::to_string(query_str[0]));
  }

  query_str.remove_prefix(1);
}

inline void exec_with(Stack& stk, bool first, bool second) {
  if (first && second) {
    stk.push(true);
  } else {
    stk.push(false);
  }
  return;
}

void AndOp::exec(Stack& stk, const ColumnCollection& _cols, const Row& _row) {
  stk.assert_not_empty();
  if (stk.top_is_bool()) return exec_with(stk,
                                          stk.pop_bool(),
                                          stk.pop_bool());
  throw ImplementationError("can't boolean and on strings");
}
