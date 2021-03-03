#include "chunk_vec.hpp"

#include <limits.h>
#include <unistd.h>
#include <sys/uio.h>

#include "error.hpp"

void ChunkVec::push_back(Chunk&& value) {
  if (value.is_empty()) return;

  return _ChunkVec_base::push_back(std::move(value));
}

bool ChunkVec::write(int fd) {
  if (0 == size()) return true;

  Chunk* send_next = data();
  size_t to_send = (size() < IOV_MAX) ? size() : IOV_MAX;

  ssize_t did_send = writev(fd, data(), to_send);
  if (-1 == did_send) {
    if (EAGAIN == errno) return false;
    throw SystemError();
  }

  while (did_send > 0) {
    Chunk& next_candidate = *send_next;
    if (did_send < next_candidate.iov_len) {
      size_t going_to_fallback_write =
        next_candidate.iov_len - did_send;
      char* fallback_write_start =
        (char*)next_candidate.iov_base + did_send;
      ssize_t fallback =
        ::write(fd,
                (void*) fallback_write_start,
                going_to_fallback_write);
      if (0 > fallback) {
        if (EAGAIN == errno) return false;
        throw ::SystemError();
      }
      if (going_to_fallback_write != fallback) {
        throw ::RuntimeError("i give up");
      }
    }

    did_send -= next_candidate.iov_len;
    erase(begin());
  }

  return write(fd);
}

size_t ChunkVec::content_length() {
  size_t going = 0;
  for (Chunk chunk : *this) {
    going += chunk.iov_len;
  };

  return going;
}
