#pragma once

#include "def.hpp"
#include "fixed.hpp"
#include "../oqtopus/valuevec.hpp"

#include "filesystem"
#include <string>
#include <vector>

namespace odbc {
  class Table {
  public:
    Table(std::string table_name);

    std::string name;
    std::vector<Def> defs();
    size_t row_count();

    Fixed& reader();
  private:
    std::filesystem::path path;
    std::vector<Def> _defs;
    Fixed fixed_reader;
  };
}
