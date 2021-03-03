#include "body_reader.hpp"
#include <sstream>
using namespace request;
using namespace request::body_reader;
ContentLength::ContentLength(int descriptor, Headers hs)
    : fd(descriptor), expected_length(0) {
  auto length_header = hs.find("Content-Length");
  if (hs.end() == length_header) {
    return;
  }
  std::istringstream(length_header->second) >> expected_length;
}
std::string ContentLength::raw_body() {
  std::string buf(expected_length, '\0');
  read(fd, buf.data(), expected_length);
  return buf;
}
