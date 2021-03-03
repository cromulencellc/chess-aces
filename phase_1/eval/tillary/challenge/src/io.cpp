#include "io.hpp"

#include <array>
#include <sys/uio.h>
#include <unistd.h>

Io Io::stdio() {
  return {0, 1};
}

Io::~Io() {
  if (!disposable) return;

  if (i != o) ::close(i);
  i = -1;
  ::close(o);
  o = -1;
}

void Io::write_str(const std::string_view& str) {
  const char* cur = str.data();
  const char* end = str.data() + str.size();

  while (cur < end) {
    ssize_t got = write(o, cur, end - cur);
    if (-1 == got) throw IncompleteWriteError();
    cur += got;
    if (cur > end) throw WroteTooMuchError(cur - str.data(),
                                           str.size());
  }
}

void Io::puts(const std::string& str) {
  const char nl = '\n';
  std::array<struct iovec, 2> vx = {{{.iov_base = (void*)str.data(),
                                     .iov_len = str.size()},
                                     {.iov_base = (void*)&nl,
                                     .iov_len = sizeof(nl)}}};

  ssize_t got = writev(o, vx.data(), 2);

  if (-1 == got) throw IncompleteWriteError();
  if (str.size() + 1 < got) throw WroteTooMuchError(got,
                                                    str.size() + 1);

  if (str.size() + 1 == got) return;
  if (str.size() == got) return write_outof(nl);

  std::string remain = str.substr(got);
  return puts(remain);
}

std::string Io::read_str_len(size_t len) {
  std::string ret;
  ret.resize(len);

  ssize_t acc = 0;
  while (acc < len) {
    ssize_t got = read(i, ret.data() + acc, len - acc);
    if (0 == got) throw ReadZeroError();
    if (-1 == got) throw IncompleteReadError();
    acc += got;
    if (acc > len) throw ReadTooMuchError(len, acc);
  }

  return ret;
}

std::string Io::read_str_delim(char delim, size_t max) {
  std::string ret;
  char cur;
  while (true) {
    read_into(cur);
    ret.push_back(cur);

    if (delim == cur) return ret;
    if (ret.size() == max) return ret;
  };
}

bool Io::read_buf_delim(std::string& buf, char delim, size_t max_buf_len) {
  char cur;
  while (true) {
    if (max_buf_len == buf.size()) return false;
    try {
      read_into(cur);
    } catch (const IncompleteReadError& _ire) {
      if (EAGAIN == errno) return false;
      throw;
    }
    buf.push_back(cur);
    if (delim == cur) return true;
  }
}

bool Io::read_buf_count(std::string &buf, size_t max_buf_len) {
  char cur;

  while (true) {
    if (max_buf_len == buf.size()) return false;
    try {
      read_into(cur);
    } catch (const IncompleteReadError& _ire) {
      if (EAGAIN == errno) return false;
      throw;
    }

    buf.push_back(cur);
  }
}
