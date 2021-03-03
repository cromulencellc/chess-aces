#pragma once

#include "error.hpp"

#include <cstdint>
#include <filesystem>
#include <string>

#include <unistd.h>

class IncompleteReadError : public OvingtonSystemError {};
class IncompleteWriteError : public OvingtonSystemError {};
class CouldntOpenError : public OvingtonSystemError {};
class CouldntSeekError : public OvingtonSystemError {};

class WrongSizeReadError : public OvingtonRuntimeError {
public:
  WrongSizeReadError(size_t expected, ssize_t got) :
    OvingtonRuntimeError{"expected to read " + std::to_string(expected) +
                         " but got " + std::to_string(got)}
  {}
};

class WroteTooMuchError : public OvingtonRuntimeError {
public:
  WroteTooMuchError(size_t expected, ssize_t got) :
    OvingtonRuntimeError{"expected to write " + std::to_string(expected) +
                         " but wrote " + std::to_string(got)}
  {}
};

template <typename T>
void checked_buf_read(int fd, T& buf) {
  ssize_t acc = 0;
  while (acc < sizeof(buf)) {
    ssize_t got = read(fd, &buf + acc, sizeof(buf) - acc);
    if (-1 == got) {
      throw IncompleteReadError();
    }
    acc += got;
    if (acc > sizeof(buf)) {
      throw WrongSizeReadError(sizeof(buf), acc);
    }
  }
}

template <typename T>
void checked_write(int fd, T obj) {
  ssize_t acc = 0;
  while (acc < sizeof(obj)) {
    ssize_t got = write(fd,
                        (&obj + acc),
                        sizeof(obj) - acc);
    if (-1 == got) {
      throw IncompleteWriteError();
    }
    acc += got;
    if (acc > sizeof(obj)) {
      throw WroteTooMuchError(sizeof(obj), acc);
    }
  }
}

void checked_write_str(int fd, const std::string& str);
std::string checked_read_str_len(int fd, size_t len);
char checked_read_str_separated(int fd, std::string& str);
int checked_open_reading(std::filesystem::path filename);
int checked_open_writing(std::filesystem::path filename);
int checked_open(std::filesystem::path filename, int flags);

off_t checked_seek(int fd, off_t offset, int whence);
