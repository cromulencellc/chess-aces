#include "string.hpp"

#include "../odbc.hpp"
#include "../checked_io.hpp"

#include <cassert>
#include <fcntl.h>
#include <unistd.h>

using namespace odbc;

String::String(std::filesystem::path base_filename,
               bool wr) : writing(wr) {
#ifndef NO_FILESYSTEM
  std::filesystem::create_directories(base_filename);
#endif
  std::filesystem::path strings_filename = base_filename / "strings";

  int open_flags = O_RDONLY;
  if (writing) {
    open_flags = O_WRONLY | O_CREAT | O_EXCL;
  }

#ifndef NO_FILESYSTEM
  string_fd = checked_open(strings_filename, open_flags);
#else
  string_fd = 2; //stderr
#endif
}

String::~String() {
  close(string_fd);
}

std::string String::get(off_t offset) {
  assert( ! writing);

  off_t got_offset = lseek(string_fd, offset, SEEK_SET);
  assert(got_offset == offset);

  size_t len;
  checked_buf_read(string_fd, len);

  std::string got = checked_read_str_len(string_fd, len);
  return got;
}

off_t String::put(std::string str) {
  assert(writing);

  off_t offset = lseek(string_fd, 0, SEEK_CUR);
  assert(0 <= offset);

  size_t len = str.size();
  checked_write(string_fd, len);

  checked_write_str(string_fd, str);

  return offset;
}
