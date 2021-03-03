#include "eq.hpp"

#include "../../logger.hpp"

using namespace yaml;
using namespace yaml::opcodes;

Eq::Eq(std::string_view& query_str) {
  if (0 == query_str.size()) {
    throw ImplementationError("query string unexpectedly empty");
  }

  if ((char)Opcode::Code::eq != query_str[0]) {
    throw ImplementationError("looked for eq opcode, got " +
                              std::to_string(query_str[0]));
  }

  query_str.remove_prefix(1);
}

template <typename T>
inline void exec_with(Stack& stk, T first, T second) {
  // LLL.debug() << "eq `"
  //             << first << "` `" << second << "` "
  //             << (first == second);
  if (first == second) {
    stk.push(true);
  } else {
    stk.push(false);
  }
  return;
}

void Eq::exec(Stack& stk, const ColumnCollection& _cols, const Row& _row) {
  stk.assert_not_empty();
  if (stk.top_is_bool()) return exec_with(stk,
                                          stk.pop_bool(),
                                          stk.pop_bool());
  if (stk.top_is_string()) return exec_with(stk,
                                            stk.pop_string(),
                                            stk.pop_string());
  throw ImplementationError("tried to eq but no idea what's on the stack");
}
