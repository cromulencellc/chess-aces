#pragma once

#include "error.hpp"
#include "logger.hpp"

#include <errno.h>
#include <string>
#include <unistd.h>

class IncompleteReadError : public SystemError {};
class IncompleteWriteError : public SystemError {};
class ReadTooMuchError : public RuntimeError {
public:
  ReadTooMuchError(size_t expected, ssize_t got) :
    RuntimeError{"expected to read " + std::to_string(expected) +
                 " but read " + std::to_string(got)} {}
};
class WroteTooMuchError : public RuntimeError {
public:
  WroteTooMuchError(size_t expected, ssize_t got) :
    RuntimeError{"expected to write " + std::to_string(expected) +
                 " but wrote " + std::to_string(got)} {}
};
class ReadZeroError : public RuntimeError {
public:
  ReadZeroError() : RuntimeError{"read zero bytes, probably eof"} {}
};

class Io {
public:
  static Io stdio();
  Io(int client_fd) : i(client_fd), o(client_fd), disposable(true) {};
  Io(int in_fd, int out_fd) : i(in_fd), o(out_fd) {};
  Io(const Io& _) = delete; // don't copy, must reference
  ~Io();

  int i;
  int o;
  bool disposable = false;

  template <typename T>
  void read_into(T& buf) {
    ssize_t acc = 0;
    while (acc < sizeof(buf)) {
      ssize_t got = read(i, &buf + acc, sizeof(buf) - acc);
      if (0 == got) throw ReadZeroError();
      if (-1 == got) {
        LLL.info() << "got errno " << errno << "reading fd " << i;
        throw IncompleteReadError();

      }
      acc += got;
      if (acc > sizeof(buf)) throw ReadTooMuchError(sizeof(buf), acc);
    }
  }

  template <typename T>
  void write_outof(T& obj) {
    ssize_t acc = 0;
    while (acc < sizeof(obj)) {
      ssize_t got = write(o, &obj + acc, sizeof(obj) - acc);
      if (-1 == got) throw IncompleteWriteError();
      acc += got;
      if (acc > sizeof(obj)) throw WroteTooMuchError(sizeof(obj), acc);
    }
  }

  void write_str(const std::string_view& str);
  void puts(const std::string& str);
  std::string read_str_len(size_t len);
  std::string read_str_delim(char delim, size_t max = 256);

  // return if it was complete
  bool read_buf_delim(std::string& buf, char delim, size_t max_buf_len = 256);
  // count includes how much buf is so far
  bool read_buf_count(std::string& buf, size_t max_buf_len);
};
