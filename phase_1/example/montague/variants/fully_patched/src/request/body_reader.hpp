#pragma once
#include "headers.hpp"
namespace request {
namespace body_reader {
class ContentLength {
public:
  ContentLength(int descriptor, Headers hs);
  std::string raw_body();
private:
  int fd;
  size_t expected_length;
};
}
}
