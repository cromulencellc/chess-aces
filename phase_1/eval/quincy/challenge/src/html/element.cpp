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

Element::Element(ParseIterator& parse) {
  std::shared_ptr<Tag> maybe_opener = std::dynamic_pointer_cast<Tag>(*parse);

  if (nullptr == maybe_opener) {
    throw DidntGet("a Tag", (*parse)->inspect());
  }
  opener = *maybe_opener;
  opener.attributes();
  std::string name = opener.name;
  parse++;

  while (!parse.kicked()) {
    LLL.debug() << "parsing " << (*parse)->inspect() << " into " << name;
    if ((*parse)->closes(name)) {
      LLL.debug() << "probably a closer";
      std::shared_ptr<CloseTag> maybe_closer =
        std::dynamic_pointer_cast<CloseTag>(*parse);
      if (nullptr == maybe_closer) {
        throw DidntGet("a CloseTag", (*parse)->inspect());
      }
      closer = *maybe_closer;
      parse++;
      LLL.debug() << inspect();
      return;
    }
    NodePtr child = *parse;
    NodePtr child_parsed = child->parse(parse);
    LLL.debug() << "child parsed to " << child_parsed->inspect();
    _children.push_back(child_parsed);
  }

  throw UnexpectedlyKicked("a CloseTag");
}

NodePtr Element::parse(ParseIterator& _p) const {
  throw AlreadyParsed(inspect());
}

std::string Element::inspect() const {
  return "Element(opener: "
    + opener.inspect()
    + " closer: "
    + closer.inspect()
    + " children: "
    + std::to_string(_children.size())
    + " nodes)";
}

std::string Element::to_text() const {
  return opener.to_text() +
    _children.to_text() +
    closer.to_text();
}

void Element::pretty(PrettyPrinter& printer) const {
  opener.pretty(printer);
  printer.indent([this](PrettyPrinter& inner_printer) {
                   for (NodePtr child : _children) {
                     child->pretty(inner_printer);
                   }
                 });
  closer.pretty(printer);
}
