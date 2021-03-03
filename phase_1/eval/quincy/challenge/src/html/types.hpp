#pragma once

#include <string>

namespace html {
  using _ScanIteratorBase = std::string::const_iterator;

  class ScanIterator : public _ScanIteratorBase {
  public:
    ParseIterator(const _ScanIteratorBase inner,
                  const _ScanIteratorBase e) :
      _ScanIteratorBase(inner), end(e) {}
    ScanIterator(const ScanIterator& _) = delete;

    bool kicked() const;

    _ScanIteratorBase end;
  };


}
