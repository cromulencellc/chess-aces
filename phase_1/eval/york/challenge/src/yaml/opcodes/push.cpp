#include "push.hpp"

using namespace yaml;
using namespace yaml::opcodes;

PushTrue::PushTrue(std::string_view& query_str) {
  if (0 == query_str.size()) {
    throw ImplementationError("query string unexpectedly empty");
  }

  // not checking opcode, this ctor used more than once

  query_str.remove_prefix(1);
}

void PushTrue::exec(Stack& stk,
                    const ColumnCollection& _cols,
                    const Row& _row) {
  stk.push(to_push());
}
