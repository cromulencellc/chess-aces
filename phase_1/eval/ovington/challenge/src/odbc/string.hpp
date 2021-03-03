#pragma once

#include "../odbc.hpp"
#include "../checked_io.hpp"

#include <filesystem>
#include <string>

namespace odbc {
  class String {
  public:
    String(std::filesystem::path base_filename, bool wr);
    ~String();

    std::string get(off_t offset);
    off_t put(std::string str);
  private:
    int string_fd;
    bool writing;
  };
}
