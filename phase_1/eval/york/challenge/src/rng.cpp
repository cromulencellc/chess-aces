#include "rng.hpp"

#include <fstream>
#include <mutex>

std::ifstream base_rng("/dev/urandom", std::ios::binary);
std::mutex base_rng_mtx = {};

std::string Rng::get_printables(size_t count) {
  std::string buf;
  buf.reserve(count);
  for (size_t i = 0; i < count; i++) {
    buf.push_back(get_printable());
  }

  return buf;
}

char Rng::get_printable() {
  char val;
  base_rng_mtx.lock();
  base_rng.read(&val, sizeof(val));
  base_rng_mtx.unlock();

  char base = 'a';
  if (val & 0x10) base = 'A';

  return base + (val & 0x0f);
}
