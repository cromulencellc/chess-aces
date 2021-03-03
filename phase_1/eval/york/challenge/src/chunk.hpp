#pragma once

#include <cstdlib>
#include <cstring>
#include <sys/uio.h>
#include <utility>
#include <vector>

#include "logger.hpp"

#include "error.hpp"

class Chunk : public iovec {
public:
  Chunk() : iovec({.iov_base = nullptr,
                   .iov_len = 0}) {}

  template <typename T>
  Chunk(const std::basic_string<T>& str) :
    Chunk(sizeof(T), (void*)str.data(), str.size()) {}

  template <typename T>
  Chunk(const std::vector<T>& vec) :
    Chunk(sizeof(T), (void*)vec.data(), vec.size()) {}

  // template <typename T, size_t N>
  // Chunk(const T(&arr)[N]) :
  //   Chunk(sizeof(T), (void*)&arr, N) {}

  Chunk(const Chunk& other) :
    Chunk(1, other.iov_base, other.iov_len) {}

  bool operator==(const Chunk& other);

  // move assignment
  Chunk& operator=(Chunk&& other);

  // copy assignment
  Chunk& operator=(const Chunk& other);

  ~Chunk();

  bool is_empty() const;

private:
  Chunk(size_t element_size, void* source_base, size_t element_count);
};
