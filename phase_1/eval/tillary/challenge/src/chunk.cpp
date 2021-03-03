#include "chunk.hpp"

#include "logger.hpp"

Chunk::Chunk(size_t element_size, void* source_base, size_t element_count) {
  if (0 == element_size) {
    throw RuntimeError("couldn't make Chunk with 0 element_size");
  }

  if (0 == element_count) {
    iov_base = nullptr;
    iov_len = 0;
    return;
  }

  iov_len = element_size * element_count;
  iov_base = std::calloc(element_count, element_size);
  if (nullptr == iov_base) {
    throw SystemError();
  }

  void* memcpy_got = memcpy(iov_base, source_base, iov_len);
  if (iov_base != memcpy_got) {
    throw RuntimeError("got unexpected result from memcpy into Chunk");
  }
}

Chunk::~Chunk() {
  if (nullptr == iov_base) return;
  free(iov_base);
  iov_base = nullptr;
}

bool Chunk::is_empty() const {
  if (nullptr == iov_base) return true;
  if (0 == iov_len) return true;

  return false;
}

bool Chunk::operator==(const Chunk& other) {
  if (iov_base != other.iov_base) return false;
  if (iov_len != other.iov_len) return false;
  return true;
}

Chunk& Chunk::operator=(Chunk&& other) {
  iov_base = other.iov_base;
  other.iov_base = nullptr;

  iov_len = other.iov_len;
  other.iov_len = 0;

  return *this;
}

Chunk& Chunk::operator=(const Chunk& other) {
  Chunk(1, other.iov_base, other.iov_len);

  return *this;
}
