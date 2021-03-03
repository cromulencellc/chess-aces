#pragma once

#include "../odbc.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace odbc {
  class Def {
  public:
    static std::vector<Def> read_from_file(int defs_fd);
    static std::vector<Def> read_from_dataload(int dataload_fd);
    static std::vector<Def> read_from_file(std::filesystem::path base_filename);
    
    static void write_to_file(std::filesystem::path base_filename,
                              std::vector<Def> defs);
    static size_t row_size(std::vector<Def>);

    Def(ColumnType t, std::string n) : type(t), name(n) {};

    size_t size();
    
    ColumnType type;
    std::string name;
  };
}

std::ostream& operator<<(std::ostream& o, const std::vector<odbc::Def>& ds);
std::ostream& operator<<(std::ostream& o, const odbc::Def& d);
