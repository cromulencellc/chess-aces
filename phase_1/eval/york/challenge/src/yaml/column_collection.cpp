#include "column_collection.hpp"

#include "error.hpp"

#include <ostream>

using namespace yaml;

size_t ColumnCollection::column_index_for(const std::string& column_name) const
{
  for (size_t idx = 0; idx < size(); idx++) {
    if (column_name == at(idx)) return idx;
  }

  throw RuntimeError("couldn't find column named " + column_name);
}

std::ostream& operator<<(std::ostream& o, const yaml::ColumnCollection& cc) {
  o << "[";
  bool first_col = true;
  for (const Column col : cc) {
    if (!first_col) o << ", ";
    first_col = false;
    o << col;
  }
  o << "]";
  return o;
}
