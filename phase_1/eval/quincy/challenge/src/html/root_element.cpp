#include "node.hpp"

#include "entities.hpp"
#include "error.hpp"
#include "muncher.hpp"

#include "void_word.hpp"

#include "../logger.hpp"

#include <charconv>
#include <regex>

using namespace html;
using namespace html::node;

RootElement::RootElement(ParseIterator& parse) {
  while (!parse.kicked()) {
    _children.push_back((*parse)->parse(parse));
  }
}

NodePtr RootElement::parse(ParseIterator& parse) const {
  throw AlreadyParsed(inspect());
}

std::string RootElement::inspect() const {
  return "RootElement(children: "
    + std::to_string(_children.size())
    + " nodes)";
}

std::string RootElement::to_text() const {
  std::string dest;

  for (NodePtr child : _children) {
    dest.append(child->to_text());
  }

  return dest;
}

void RootElement::pretty(PrettyPrinter& printer) const {
  for (NodePtr child : _children) {
    child->pretty(printer);
  }
}
