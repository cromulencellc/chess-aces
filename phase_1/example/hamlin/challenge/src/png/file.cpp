#include "common.hpp"
#include "file.hpp"

using namespace png;

const std::array<byte, 8> file_header_expectation =
  {0x89, //8-bit sigil
   0x50, 0x4e, 0x47, // PNG
   0x0d, 0x0a, // dos line-ending
   0x1a, // dos eof
   0x0a}; // unix line-ending
const uint8_t file_header_size = file_header_expectation.size();

File::File(std::istream& r) {
  std::array<byte, file_header_size> signature;
  r.read((char*)(void*) signature.data(), signature.size());

  assert(file_header_expectation == signature);
}
