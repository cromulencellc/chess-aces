#include "Effect.h"
std::ostream &Effect::serialize(std::ostream &out) const {
  out << this->priority;
  return out;
}
std::istream &Effect::dserialize(std::istream &in) {
  ISTREAM_READ(in, this->priority, "Detected EOF early in effect\n");
  return in;
}