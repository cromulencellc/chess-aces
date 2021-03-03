#pragma once

#include <vector>

#include "node_ptr.hpp"
#include "node_vec.hpp"

namespace html {
  using _ParseVec = NodeVec;
  using _ParseIteratorBase = _ParseVec::const_iterator;

  class ParseIterator : public _ParseIteratorBase {
  public:
    ParseIterator(const _ParseIteratorBase inner,
                  const _ParseIteratorBase e) :
      _ParseIteratorBase(inner), end(e) {}
    ParseIterator(const ParseIterator& _) = delete;

    bool kicked() const;
  private:
    _ParseIteratorBase end;
  };
}
