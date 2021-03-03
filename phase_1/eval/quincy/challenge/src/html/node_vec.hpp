#pragma once

#include "node_ptr.hpp"

#include <vector>

namespace html {
  using _NodeVec_Base = std::vector<NodePtr>;

  class NodeVec : public _NodeVec_Base {
  public:
    NodeVec() : _NodeVec_Base() {}

    template <typename T>
    NodeVec(T x) : _NodeVec_Base(x) {}

    std::string inspect() const;
    std::string to_text() const;
  };
}
