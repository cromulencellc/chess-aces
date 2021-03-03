#pragma once

#include <string>

#include "scan_iterator.hpp"

namespace html {
  // https://html.spec.whatwg.org/multipage/syntax.html#the-doctype
  class Doctype {
  public:
    Doctype() : valid(false) {};
    Doctype(ScanIterator& reader);

    bool valid;
    bool legacy;
  };
}
