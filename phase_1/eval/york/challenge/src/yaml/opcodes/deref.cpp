#include "deref.hpp"

using namespace yaml;
using namespace yaml::opcodes;

Deref::Deref(std::string_view& query_str) {
  if (0 == query_str.size()) {
    throw ImplementationError("query string unexpectedly empty");
  }

  if ((char)Opcode::Code::deref != query_str[0]) {
    throw ImplementationError("looked for deref opcode, got " +
                              std::to_string(query_str[0]));
  }

  query_str.remove_prefix(1);
}

void Deref::exec(Stack& stk, const ColumnCollection& cols, const Row& row) {
  if (false == did_cache_column_number) {
    return exec_without_cache(stk, cols, row);
  }

  std::string current_column_name = stk.pop_string();
  if (current_column_name != cached_column_name) {
    // asking for a different column name than what's cached
    return exec_without_cache(stk, cols, row, current_column_name);
  }

  if (current_column_name != cols[cached_column_number]) {
    // asking for the same column but the number changed somehow?
    return exec_without_cache(stk, cols, row, current_column_name);
  }

  std::string value = row[cached_column_number];
  stk.push(value);
}

void Deref::exec_without_cache(Stack& stk,
                        const ColumnCollection& cols,
                        const Row& row) {
  std::string current_column_name = stk.pop_string();
  return exec_without_cache(stk, cols, row, current_column_name);
}


void Deref::exec_without_cache(Stack& stk,
                               const ColumnCollection& cols,
                               const Row& row,
                               const std::string& column_name) {
  cached_column_number = get_column_number(cols, column_name);
  cached_column_name = column_name;
  did_cache_column_number = true;

  std::string value = row[cached_column_number];
  stk.push(value);
}

size_t Deref::get_column_number(const ColumnCollection& cols,
                         const std::string& column_name) {
  for (size_t col_num = 0; col_num < cols.size(); col_num++) {
    const Column& col = cols[col_num];
    if (column_name == col) return col_num;
  }

  throw UnknownColumnError(column_name);
}
