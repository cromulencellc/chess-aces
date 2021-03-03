#include "def.hpp"

#include "../ansi.hpp"
#include "../checked_io.hpp"
#include "../crc32.hpp"
#include "../float.hpp"

#include <array>
#include <cassert>

using namespace std::literals::string_literals;
using namespace odbc;


std::vector<Def> Def::read_from_file(std::filesystem::path base_filename) {
  int def_reader = checked_open_reading(base_filename / "defs"s);
  std::vector<Def> got = read_from_file(def_reader);
  close(def_reader);
  return got;
}

std::vector<Def> Def::read_from_file(int defs_fd) {
  std::vector<Def> running = {};

  while (true) {
    char sigil;
    ssize_t got = read(defs_fd, &sigil, sizeof(sigil));
    if (0 == got) return running;
    if (0 > got) throw IncompleteReadError();

    size_t name_size;
    checked_buf_read(defs_fd, name_size);

    std::string nombre = checked_read_str_len(defs_fd, name_size);

    running.push_back({(ColumnType) sigil, nombre});
  }
}

std::vector<Def> Def::read_from_dataload(int dataload_fd) {
  std::vector<Def> running = {};

  while (true) {
    char sigil;
    checked_buf_read(dataload_fd, sigil);

    std::string name;
    char next_def_sigil = checked_read_str_separated(dataload_fd, name);

    running.push_back({(ColumnType) sigil, name});

    if (ansi::separator::group == next_def_sigil) return running;
    if (ansi::separator::unit != next_def_sigil) {
      throw OdbcError("unexpected delimiter " +
                      std::to_string((int) next_def_sigil));
    }
  }
}

void Def::write_to_file(std::filesystem::path base_filename,
                        std::vector<Def> defs) {
#ifndef NO_FILESYSTEM
  std::filesystem::create_directories(base_filename);
#endif
  std::filesystem::path defs_filename = base_filename / "defs"s;

#ifndef NO_FILESYSTEM
  int defs_fd = checked_open_writing(defs_filename);
#else
  int defs_fd = 2; // stderr
#endif
  for (Def d : defs) {
    checked_write(defs_fd, (char)d.type);
    size_t name_size = d.name.size();
    checked_write(defs_fd, name_size);
    checked_write_str(defs_fd, d.name);
  }
}

size_t Def::size() {
  switch (type) {
  case ColumnType::sint64:
    return sizeof(int64_t); // yolo = you oughta look out
  case ColumnType::float64:
    return sizeof(float64_t);
  case ColumnType::stringz:
    return sizeof(off_t);
  }
}

size_t Def::row_size(std::vector<Def> defs) {
  size_t running = 0;
  running += sizeof(size_t); // rownum
  for (Def d : defs) {
    running += d.size();
  }
  running += sizeof(crc32::crc_t);
  return running;
}

std::ostream& operator<<(std::ostream& o, const odbc::Def& d) {
  o << "odbc::Def(" << (char)d.type << " " << d.name << ") ";
  return o;
}

std::ostream& operator<<(std::ostream& o, const std::vector<Def>& ds) {
  o << "std::vector<Def> {" << std::endl;
  for (Def d : ds) {
    o << "\t" << d << std::endl;
  }
  o << "}" << std::endl;
  return o;
}
