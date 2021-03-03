#include "not_op.hpp"

using namespace yaml;
using namespace yaml::opcodes;

NotOp::NotOp(std::string_view& query_str) {
  if (0 == query_str.size()) {
    throw ImplementationError("query string unexpectedly empty");
  }

  if ((char)Opcode::Code::not_op != query_str[0]) {
    throw ImplementationError("looked for not_op opcode, got " +
                              std::to_string(query_str[0]));
  }

  query_str.remove_prefix(1);
}

void NotOp::exec(Stack& stk, const ColumnCollection& _cols, const Row& _row) {
  stk.assert_not_empty();
  if (!stk.top_is_bool()) {
    throw ImplementationError("can't boolean not on strings");
  }

  stk.push(! stk.pop_bool());
}
