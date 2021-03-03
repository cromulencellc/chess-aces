#include "headers.hpp"
std::ostream &operator<<(std::ostream &o, response::Headers &hs) {
  o << "Headers(" << std::endl;
  for (response::Header hdr : hs) {
    o << "\t" << hdr.first << ": " << hdr.second << std::endl;
  }
  return o;
}
