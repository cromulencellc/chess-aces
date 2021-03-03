#pragma once

#include <vector>
#include "column.hpp"

namespace yaml {
  using _ColumnCollection_Base = std::vector<Column>;

  class ColumnCollection : public _ColumnCollection_Base {
  public:
    size_t column_index_for(const std::string& column_name) const;
  };
}

std::ostream& operator<<(std::ostream& o, const yaml::ColumnCollection& cc);
