#include "fixed.hpp"

#include "../ansi.hpp"
#include "../checked_io.hpp"

#include "../oqtopus/environment.hpp"

#include <cassert>
#include <fcntl.h>
#include <unistd.h>

using namespace odbc;

namespace value = oqtopus::value;

Fixed::Fixed(std::filesystem::path base_filename,
             std::vector<Def> dfs,
             bool wr) :
  string(base_filename, wr),
  writing(wr),
  defs(dfs),
  row_number(0),
  _row_count(0)
{
#ifndef NO_FILESYSTEM
  std::filesystem::create_directories(base_filename);
#endif
  std::filesystem::path fixed_filename = base_filename / "fixeds";

  int open_flags = O_RDONLY;
  if (writing) {
    open_flags = O_WRONLY | O_CREAT | O_EXCL;
  }
#ifndef NO_FILESYSTEM
  fixed_fd = checked_open(fixed_filename, open_flags);
#else
  fixed_fd = 2; // stderr
#endif
}

Fixed::~Fixed() {
  close(fixed_fd);
}

void Fixed::dataload_rows(int in_fd, int out_fd) {
  assert(writing);
  crc32::crc_t outer_ck = crc32::calculate_begin();

  while (true) {
    crc32::crc_t ck =
      crc32::calculate_begin((const char*)&row_number, sizeof(row_number));
    char sigil;
#ifndef NO_FILESYSTEM
    checked_write(fixed_fd, row_number);
#endif
    row_number++;

    for (Def d : defs) {
      std::string buf;
      byte len;
      off_t pos;
      switch (d.type) {
      case ColumnType::sint64:
      case ColumnType::float64:
        buf = checked_read_str_len(in_fd, 8);
        assert(8 == buf.size());
        ck = crc32::calculate_inter(ck, buf.data(), buf.size());
#ifndef NO_FILESYSTEM
        checked_write_str(fixed_fd, buf);
#endif
        break;
      case ColumnType::stringz:
        checked_buf_read(in_fd, len);
        buf = checked_read_str_len(in_fd, len);
        assert(len == buf.size());
        ck = crc32::calculate_inter(ck, buf.data(), buf.size());
#ifndef NO_FILESYSTEM
        pos = string.put(buf);
        checked_write(fixed_fd, pos);
#endif
        break;
      }
    }
    ck = crc32::calculate_final(ck);
#ifndef NO_FILESYSTEM
    checked_write(fixed_fd, ck);
#endif
    // LLL.debug() << std::hex << "row " << (row_number - 1) << " crc " << ck;

    outer_ck = crc32::calculate_inter(outer_ck, (const char*)&ck, sizeof(ck));

    checked_buf_read(in_fd, sigil);

    if (ansi::separator::file == sigil) break;
    if (ansi::separator::record != sigil) {
      throw OdbcError("unepxected sigil " + std::to_string(sigil));
    }
  }

  crc32::crc_t outer_ck_final = crc32::calculate_final(outer_ck);

  LLL.debug() << "loaded " << row_number << " rows outer_ck "
              << std::hex << outer_ck_final;

  checked_write(out_fd, outer_ck_final);

  _row_count = row_number;
}

void Fixed::rewind() {
  row_number = 0;
  off_t got = checked_seek(fixed_fd, 0, SEEK_SET);
  assert(0 == got);
}

size_t Fixed::row_count() {
  if (_row_count != 0) return _row_count;

  off_t was_at = checked_seek(fixed_fd, 0, SEEK_CUR);

  off_t total_len = checked_seek(fixed_fd, 0, SEEK_END);

  off_t got_was = checked_seek(fixed_fd, was_at, SEEK_SET);
  assert(was_at == got_was);

  size_t row_size = Def::row_size(defs);
  assert(0 == (total_len % row_size));
  _row_count = total_len / row_size;

  return _row_count;
}

bool Fixed::more_to_read() {
  return row_number < row_count();
}

void Fixed::read_into_env(oqtopus::Environment& env) {
  assert(!writing);
  size_t got_row_number;
  checked_buf_read(fixed_fd, got_row_number);
  assert(row_number == got_row_number);
  crc32::crc_t ck =
    crc32::calculate_begin((const char*)&row_number, sizeof(row_number));

  for (Def d : defs) {
    int64_t got_i;
    float64_t got_f;
    off_t str_pos;
    std::string got_s;
    switch (d.type) {
    case ColumnType::sint64:
      checked_buf_read(fixed_fd, got_i);
      ck = crc32::calculate_inter(ck, (const char*)&got_i, sizeof(got_i));
      env.set(d.name,
              std::make_shared<value::Sint64>((unsigned long long)got_i));
      break;
    case ColumnType::float64:
      checked_buf_read(fixed_fd, got_f);
      ck = crc32::calculate_inter(ck, (const char*)&got_f, sizeof(got_f));
      env.set(d.name,
              std::make_shared<value::Float64>(got_f));
      break;
    case ColumnType::stringz:
      checked_buf_read(fixed_fd, str_pos);
      got_s = string.get(str_pos);
      ck = crc32::calculate_inter(ck, got_s.data(), got_s.size());
      env.set(d.name,
              std::make_shared<value::Stringz>(got_s));
      break;
    }
  }

  ck = crc32::calculate_final(ck);
  crc32::crc_t got_ck;
  checked_buf_read(fixed_fd, got_ck);
  assert(ck == got_ck);

  row_number++;
}
