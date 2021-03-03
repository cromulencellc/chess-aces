#include "uuid.hpp"

#include <fstream>
#include <mutex>

std::ifstream uuid_rng("/dev/urandom", std::ios::binary);
std::mutex uuid_rng_mtx = {};

std::string UUID_FORMAT = "%08x-%04x-%04x-%02x%02x-%012lx";

Uuid::Uuid() {
  std::string buf = "12345678";
  uuid_rng_mtx.lock();
  uuid_rng.read(reinterpret_cast<char*>(this),
           sizeof(Uuid));
  uuid_rng_mtx.unlock();

  version_and_time_high >>= 4;
  version_and_time_high |= (4 << 12);
  variant_and_clock_seq_high >>= 2;;
  variant_and_clock_seq_high |= (2 << 6);
}

Uuid::Uuid(std::string hex_encoded) {
  sscanf(hex_encoded.c_str(), UUID_FORMAT.c_str(),
         &time_low,
         &time_mid,
         &version_and_time_high,
         &variant_and_clock_seq_high, &clock_seq_low,
         &node);
}

std::string to_string(const Uuid& u) {
  std::string buf = "12345678-1234-5678-1234-567812345678";

  snprintf(buf.data(), buf.size() + 1,
           UUID_FORMAT.data(),
           u.time_low,
           u.time_mid,
           u.version_and_time_high,
           u.variant_and_clock_seq_high, u.clock_seq_low,
           u.node);

  return buf;
}

std::ostream& operator<<(std::ostream& o, const Uuid& u) {
  o << to_string(u);
  return o;
}
