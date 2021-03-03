#include "table.hpp"

#include "error.hpp"

#include "../logger.hpp"

#include <filesystem>

using namespace yaml;

const std::filesystem::path tables_base = "/data/";
const size_t max_cell_size = 512;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-const-variable"
const char fs = 0x1c; // file
const char gs = 0x1d; // group
const char rs = 0x1e; // record/row
const char us = 0x1f; // unit/cell
#pragma GCC diagnostic pop

const Row empty_row = {};

Table::Table(std::string filename) {
  std::filesystem::path path =
    (tables_base / std::filesystem::path(filename)).replace_extension(".yaml");
  reader = {path};

  reader.seekg(0, std::ios_base::end);
  total_file_size = reader.tellg();
  reader.seekg(0);

  LLL.debug() << "opened " << path
              << " offset " << reader.tellg()
              << " out of " << total_file_size;
}

const ColumnCollection& Table::get_columns() {
  if (done_reading_columns) return columns;

  if (0 != reader.tellg()) {
    throw RuntimeError("expected to read columns from byte zero of yaml file");
  }

  while (true) {
    std::string cell = read_cell();
    columns.push_back(cell);
    LLL.debug() << "column `" << cell << "`";
    if (gs == reader.peek()) break;
  }

  char got = reader.get();
  if (gs != got) {
    throw RuntimeError("was done reading columns but got non-gs byte");
  }

  done_reading_columns = true;

  return columns;
}

size_t Table::column_index_for(const std::string& column_name) {
  const ColumnCollection& cols = get_columns();
  size_t idx = 0;
  bool did_find_column = false;
  for (const Column c : cols) {
    if (column_name == c) {
      did_find_column = true;
      break;
    }
    idx++;
  }

  if (! did_find_column) {
    throw RuntimeError("couldn't find column named " + column_name);
  }

  return idx;
}

std::unique_ptr<Row> Table::find_first_row(const Program& prog) {
  size_t cur_row = 0;

  while (true) {
    const Row& found = row_for_cursor(cur_row);
    if (0 == found.size()) break; // ran out of rows

    if (prog.matches(get_columns(), found)) {
      LLL.debug() << "matched!";
      return std::make_unique<Row>(found);
    }

    cur_row++;
  }

  return nullptr;
}

std::unique_ptr<Row> Table::find_next_row(const Program& prog) {
  while (true) {
    const Row& found = row_for_cursor(find_next_row_cursor);
    find_next_row_cursor++;
    LLL.info() << found.size();
    if (0 == found.size()) break;

    if (prog.matches(get_columns(), found)) {
      return std::make_unique<Row>(found);
    }

  }

  return nullptr;
}

const Row& Table::row_for_cursor(size_t csr) {
  LLL.debug() << "want row " << csr << " of " << rows.size();
  if (csr < rows.size()) return rows[csr];

  if (did_read_all_rows && (csr >= rows.size())) return empty_row;

  Row cur;
  for (Column _col : get_columns()) {
    std::string cell = read_cell();
    // LLL.debug() << _col << ": " << cell;
    cur.push_back(cell);
  }

  char got = reader.get();
  if (rs != got) {
    throw RuntimeError("was done reading a row but got non-rs byte");
  }

  reader.peek();

  if (reader.eof() || (reader.tellg() == total_file_size)) {
    did_read_all_rows = true;
  }

  rows.push_back(cur);

  return row_for_cursor(csr);
}

std::string Table::read_cell() {
  std::string buf;

  buf.resize(max_cell_size, '\0');
  size_t col_start = reader.tellg();
  reader.getline(buf.data(), max_cell_size, us);
  if (!reader.good()) {
    throw RuntimeError("reader not good while reading cell");
  }
  size_t col_end = reader.tellg();
  buf.resize(col_end - (col_start + 1)); // +1 to eat the unit separator
  return buf;
}

std::ostream& operator<<(std::ostream& o, const yaml::Table& t) {
  o << "yaml::Table("
    << "done_reading_columns="
    << (t.done_reading_columns ? "true" : "false")
    << " did_read_all_rows="
    << (t.did_read_all_rows ? "true" : "false")
    << " columns="
    << t.columns
    << ")";

  return o;
}
