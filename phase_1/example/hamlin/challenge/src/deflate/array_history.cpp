#include "array_history.hpp"

using namespace deflate;

ArrayHistory::ArrayHistory(ZlibHeader& zlh) : header(zlh) {
  assert(header.window_size() == buf.size());
}

void ArrayHistory::append(byte b) {
  buf[cursor] = b;
  if (cursor +1 >= buf.size()) {
    wrapped = true;
  }
  cursor = (cursor + 1) % buf.size();
}

std::vector<byte> ArrayHistory::copy(uint32_t dist, uint16_t count) {
  std::ptrdiff_t start_cur = (cursor - dist);

  std::vector<byte> cpy{};
  cpy.reserve(count);

  for (std::size_t n = 0; n < count; n++) {
    std::ptrdiff_t pos = n + start_cur;
#ifndef PATCH_ARRAYHISTORY_NEGATIVE_IDX
    if (wrapped) {
      pos = pos % buf.size();
    }
#else
    pos = pos % buf.size();
#endif
    byte b = buf[pos];
    append(b);
    cpy.push_back(b);
  }

  return cpy;
}
