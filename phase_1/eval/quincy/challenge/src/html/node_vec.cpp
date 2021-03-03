#include "node_vec.hpp"

#include "node.hpp"

#include <sstream>

using namespace html;

std::string NodeVec::inspect() const {
  std::stringstream buf;

  buf << "NodeVec(\n";
  for (NodePtr p : *this) {
    buf << "\t" << p->inspect() << "\n";
  }
  buf << ")";

  return buf.str();
}

std::string NodeVec::to_text() const {
  std::stringstream buf;

  for (NodePtr p : *this) {
    buf << p->to_text();
  }

  return buf.str();
}
