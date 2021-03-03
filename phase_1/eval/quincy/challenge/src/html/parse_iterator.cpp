#include "parse_iterator.hpp"

using namespace html;

bool ParseIterator::kicked() const {
  return end <= *this;
}
