#include "checked_io.hpp"

#include "ansi.hpp"

#include <fcntl.h>

void checked_write_str(int fd, const std::string& str) {
  ssize_t tot = str.size();
  const char* start = str.data();
  ssize_t acc = 0;
  while (acc < tot) {
    ssize_t got = write(fd,
                        start + acc,
                        tot - acc);
    if (-1 == got) {
      throw IncompleteWriteError();
    }
    acc += got;
    if (acc > tot) {
      throw WroteTooMuchError(tot, acc);
    }
  }
}

std::string checked_read_str_len(int fd, size_t len) {
  std::string ret;
  ret.resize(len);

  ssize_t acc = 0;
  while (acc < len) {
    ssize_t got = read(fd, ret.data() + acc, len - acc);
    if (-1 == got) {
      throw IncompleteReadError();
    }
    acc += got;
    if (acc > len) {
      throw WrongSizeReadError(len, acc);
    }
  }

  return ret;
}

char checked_read_str_separated(int fd, std::string& dest) {
  char buf;

  while (true) {
    checked_buf_read(fd, buf);

    if ((ansi::separator::file <= buf) &&
        (buf <= ansi::separator::unit)) {
      return buf;
    }

    dest.push_back(buf);
  }
}

int checked_open_reading(std::filesystem::path filename) {
  return checked_open(filename, O_RDONLY);
}

int checked_open_writing(std::filesystem::path filename) {
  return checked_open(filename, O_WRONLY | O_CREAT | O_TRUNC);
}

int checked_open(std::filesystem::path filename, int flags) {
  int got_fd = open(filename.c_str(), flags);
  if (got_fd < 0) throw CouldntOpenError();
  return got_fd;
}

off_t checked_seek(int fd, off_t offset, int whence) {
  off_t got = lseek(fd, offset, whence);

  if (-1 == got) throw CouldntSeekError();
  return got;
}
