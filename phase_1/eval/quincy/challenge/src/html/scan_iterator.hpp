#pragma once

#include <string>

namespace html {
  using _ScanIteratorBase = std::string::const_iterator;

  class ScanIterator : public _ScanIteratorBase {
  public:
    ScanIterator(const _ScanIteratorBase inner,
                  const _ScanIteratorBase e) :
      _ScanIteratorBase(inner), end(e) {}
    ScanIterator(const ScanIterator& _) = delete;

    bool kicked() const;
    void expect_not_kicked() const;

    bool has(size_t n) const;

    void eat(std::string want);
    void eati(std::string want);

    bool can_eati(std::string want);
    bool can_eat(std::string want);

    std::string scan_to(char delim);

  private:
    _ScanIteratorBase end;
  };
}
