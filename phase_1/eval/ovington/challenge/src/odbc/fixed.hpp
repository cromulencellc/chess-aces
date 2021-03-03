#pragma once

#include "../odbc.hpp"

#include "def.hpp"
#include "string.hpp"

#include "../checked_io.hpp"
#include "../crc32.hpp"

#include <filesystem>
#include <vector>

namespace oqtopus {
  class Environment; // don't need to pull all of oqtopus into all of odbc
}

namespace odbc {
  class Fixed {
  public:
    Fixed(std::filesystem::path base_filename,
          std::vector<Def> defs,
          bool wr);
    ~Fixed();

    void dataload_rows(int in_fd, int out_fd);

    size_t row_count();

    void rewind();

    bool more_to_read();
    void read_into_env(oqtopus::Environment& env);

  private:
    int fixed_fd;
    String string;
    bool writing;
    std::vector<Def> defs;
    size_t row_number;
    size_t _row_count;
  };
}
