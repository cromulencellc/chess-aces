#include "headers.hpp"
using namespace request;
Headers::Headers(Reader &in) : _Headers_base_class{} {
  char cr = in.get();
  if ('\r' != cr)
    return;
  char lf = in.get();
  if ('\n' != lf)
    return;
  while (true) {
    if ('\r' == in.peek()) {
      cr = in.get();
      if ('\r' != cr)
        break;
      lf = in.get();
      if ('\n' != lf)
        break;
      valid = true;
      break;
    }
    Header h = Header::parse(in);
    if (!h.valid)
      break;
    insert(h);
  }
}
std::ostream &operator<<(std::ostream &o, request::Headers &hs) {
  o << "request::Headers(" << std::endl;
  for (Header h : hs) {
    o << "\t" << h << std::endl;
  }
  o << ")" << std::endl;
  return o;
}
