#pragma once
#include <cstdint>
#include <ostream>
#include <string>
class Uuid {
public:
  Uuid();
  Uuid(std::string hex_encoded);
  uint32_t time_low;
  uint16_t time_mid;
  uint16_t version_and_time_high;
  uint8_t variant_and_clock_seq_high;
  uint8_t clock_seq_low;
  uint64_t node;
};
std::string to_string(const Uuid &u);
std::ostream &operator<<(std::ostream &o, const Uuid &u);
