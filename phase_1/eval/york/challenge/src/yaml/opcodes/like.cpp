#include "like.hpp"

#include "../../logger.hpp"

using namespace yaml;
using namespace yaml::opcodes;

Like::Like(std::string_view& query_str) {
  if (0 == query_str.size()) {
    throw ImplementationError("query string unexpectedly empty");
  }

  if ((char)Opcode::Code::like != query_str[0]) {
    throw ImplementationError("looked for like opcode, got " +
                              std::to_string(query_str[0]));
  }

  query_str.remove_prefix(1);
}

template <typename T>
inline void exec_with(Stack& stk, T needle, T haystack) {
  auto got = haystack.find(needle);
  LLL.debug() << "haystack `" << haystack
              << "` needle `" << needle
              << "` find? " << (got != std::string::npos);
  if (got != std::string::npos) {
    stk.push(true);
  } else {
    stk.push(false);
  }
  return;
}

void Like::exec(Stack& stk, const ColumnCollection& _cols, const Row& _row) {
  stk.assert_not_empty();
  std::string needle = stk.pop_string();
  std::string haystack = stk.pop_string();
  return exec_with(stk, needle, haystack);
}
