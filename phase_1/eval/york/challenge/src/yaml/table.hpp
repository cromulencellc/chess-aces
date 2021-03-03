#pragma once

#include "row_collection.hpp"
#include "column_collection.hpp"

#include "program.hpp"

#include <fstream>
#include <memory>
#include <string>

namespace yaml { class Table; }

std::ostream& operator<<(std::ostream& o, const yaml::Table& t);

namespace yaml {
  class Table {
    friend std::ostream& ::operator<<(std::ostream& o, const yaml::Table& t);

  public:
    Table(std::string filename);

    const ColumnCollection& get_columns();

    std::unique_ptr<Row> find_first_row(const Program& prog);
    std::unique_ptr<Row> find_next_row(const Program& prog);

    size_t column_index_for(const std::string& column_name);


  private:
    std::ifstream reader;

    bool done_reading_columns = false;
    bool did_read_all_rows = false;

    size_t find_next_row_cursor = 0;

    size_t total_file_size = 0;

    const Row& row_for_cursor(size_t csr);

    std::string read_cell();

    ColumnCollection columns;
    RowCollection rows;
  };
}
