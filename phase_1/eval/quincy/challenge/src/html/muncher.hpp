#pragma once

#include "scan_iterator.hpp"

namespace html {
  class Muncher {
  public:
    static void whitespace(ScanIterator& s, bool mandatory = false);
    static void comments_and_whitespace(ScanIterator& s);
    static void comment(ScanIterator& s);
  };
}
