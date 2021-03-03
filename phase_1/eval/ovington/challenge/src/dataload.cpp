#include "dataload.hpp"

#include "ansi.hpp"
#include "checked_io.hpp"

#include "odbc/fixed.hpp"

#include <filesystem>
#include <string>

const std::filesystem::path data_path = "/data";

void Dataload::do_dl(int in_fd, int out_fd) {
  Dataload _dl = {in_fd, out_fd};
}

Dataload::Dataload(int in_fd, int out_fd) {
  std::string got_name = to_string(name);
  checked_write_str(out_fd, got_name);

  base_filename = data_path / got_name;

  try {
    defs = odbc::Def::read_from_dataload(in_fd);
    LLL.debug() << defs;
#ifndef NO_FILESYSTEM
    odbc::Def::write_to_file(base_filename, defs);
#endif
    odbc::Fixed fixed(base_filename, defs, true);

    fixed.dataload_rows(in_fd, out_fd);

  } catch (const odbc::OdbcError& e) {
    checked_write_str(out_fd, e.what());
    throw;
  }

}
