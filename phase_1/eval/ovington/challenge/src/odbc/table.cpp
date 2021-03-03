#include "table.hpp"

using namespace std::literals::string_literals;
using namespace odbc;

Table::Table(std::string table_name) :
  name(table_name),
  path(std::filesystem::path("/data") / table_name),
  _defs(Def::read_from_file(path)),
  fixed_reader(path, _defs, false)
{}

size_t Table::row_count() {
  return fixed_reader.row_count();
}

std::vector<Def> Table::defs() {
  return _defs;
}

Fixed& Table::reader() {
  return fixed_reader;
}
