#include "push_str.hpp"

using namespace yaml;
using namespace yaml::opcodes;

PushStr::PushStr(std::string_view& query_str) {
  if (0 == query_str.size()) {
    throw ImplementationError("query string unexpectedly empty");
  }

  if (2 > query_str.size()) {
    throw SyntaxError("query string shorter than complete push_tr op");
  }

  if ((char)Opcode::Code::push_str != query_str[0]) {
    throw ImplementationError("looked for push_str opcode, got " +
                              std::to_string(query_str[0]));
  }

  if ('\0' == query_str[1]) {
    immediate = "";
    query_str.remove_prefix(2);
    return;
  }

  query_str.remove_prefix(1);

  size_t end_idx = 0;
  bool did_find_terminator = false;

  while (end_idx < query_str.size()) {
    end_idx++;
    if ('\0' == query_str[end_idx]) {
      did_find_terminator = true;
      break;
    }
  }

  if (false == did_find_terminator) {
    throw SyntaxError("push_str couldn't find terminator");
  }

  immediate = {query_str.data(), end_idx};

  query_str.remove_prefix(end_idx + 1);
}

void PushStr::exec(Stack& stk, const ColumnCollection& _cols, const Row& _row) {
  stk.push(immediate);
}
