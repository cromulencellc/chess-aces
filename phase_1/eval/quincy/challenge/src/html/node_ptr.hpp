#pragma once

#include <memory>

namespace html {
  namespace node {
    class Base;
  }
  using Node = node::Base;
  using NodePtr = std::shared_ptr<Node>;
}
