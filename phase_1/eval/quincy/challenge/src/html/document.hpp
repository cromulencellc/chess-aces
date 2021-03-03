#pragma once

#include <string>

#include "doctype.hpp"
#include "node.hpp"

namespace html {
  class Document {
  public:
    Document(std::string body);

    Doctype doctype;
    NodePtr root;

    void pretty(Io& io) const;
  };
}
