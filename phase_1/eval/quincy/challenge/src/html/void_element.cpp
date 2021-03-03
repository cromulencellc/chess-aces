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

VoidElement::VoidElement(ParseIterator& parse) {
  std::shared_ptr<Tag> maybe_opener =
    std::dynamic_pointer_cast<Tag>(*parse);

  if (nullptr == maybe_opener) {
    throw DidntGet("a Tag", (*parse)->inspect());
  }
  opener = *maybe_opener;
  opener.attributes();
  parse++;
  LLL.debug() << inspect();
}

NodePtr VoidElement::parse(ParseIterator& _p) const {
  throw AlreadyParsed(inspect());
}

std::string VoidElement::inspect() const {
  return "VoidElement(opener: "
    + opener.inspect()
    + ")";
}

std::string VoidElement::to_text() const {
  return opener.to_text();
}

void VoidElement::pretty(PrettyPrinter& printer) const {
  opener.pretty(printer);
}
