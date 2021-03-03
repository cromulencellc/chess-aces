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

Tag::Tag(ScanIterator& scan) {
  scan.eat("<");
  char cur;
  while ((cur = *scan)
         && (!std::isspace(cur))
         && ('>' != cur)
         && ('/' != cur)) {
    name.push_back(cur);
    scan++;
    scan.expect_not_kicked();
  }

  if ('>' == cur) {
    is_self_closing = is_void_element_name(name);
    scan.eat(">");;
    return;
  }

  if ('/' == cur) {
    is_self_closing = true;
    scan.eat(">");
    return;
  }

  if (! std::isspace(cur)) {
    throw NeededWS(cur);
  }

  while (std::isspace(*scan)) scan++;

  while ((cur = *scan)
         && ('>' != cur)) {
    attribute_text.push_back(cur);
    scan++;
  }

  scan++;

  if (!is_self_closing) {
    is_self_closing = is_void_element_name(name);
  }
}

NodePtr Tag::parse(ParseIterator& p) const {
  if (true == is_self_closing) return parse_void_element(p);

  return parse_normal_element(p);
}

NodePtr Tag::parse_void_element(ParseIterator& p) const {
  assert_parsing_self(p);
  return mks(VoidElement(p));
}

NodePtr Tag::parse_normal_element(ParseIterator& p) const {
  assert_parsing_self(p);
  return mks(Element(p));
}

std::string Tag::inspect() const {
  std::string buf = "Tag(name: `"
    + name
    + "` self_closing: "
    + std::to_string(is_self_closing);

  if (did_parse_attributes) {
    buf + " attributes: (";

    for (Attribute a : _attributes) {
      buf += a.to_text(attribute_text);
      buf += " ";
    }
  } else {
    buf += " attribute_text: `"
      + attribute_text
      + "` ";
  }

  return buf + ")";
}

std::string Tag::to_text() const {
  return "<"
    + name + " "
    + attribute_text + ">";
}

void Tag::pretty(PrettyPrinter& printer) const {
  std::vector<std::string> held_units;
  WrapVec units;
  units.push_back("<");
  units.push_back(name);

  if ((did_parse_attributes && (0 < _attributes.size())) ||
      ! did_parse_attributes) {
    units.push_back(" ");
  }

  if (! did_parse_attributes) {
    units.push_back("__attributes=\"unparsed\"");
  } else {
    for (Attribute a : _attributes) {
      held_units.push_back(a.to_text(attribute_text));
    }

    for (const std::string& u : held_units) {
      units.push_back(u);
      units.push_back(" ");
    }
  }
  units.push_back(">");

  printer.wrap(units);
}

std::vector<Attribute> Tag::attributes() {
  if (did_parse_attributes) return _attributes;

  LLL.debug() << "parsing attributes `" << attribute_text <<
    "` @ " <<
    std::hex << (uint64_t)attribute_text.data();

  ssize_t attr_scan = 0;
  const ssize_t attr_end = attribute_text.size();

  while (attr_scan < attr_end) {
    Attribute nuevo = Attribute(attribute_text, attr_scan, attr_end);
    //    LLL.debug() << nuevo << nuevo.to_text(attribute_text);
    if (!nuevo.valid) {
      break;
    }
    _attributes.push_back(nuevo);
  }

  did_parse_attributes = true;
  return _attributes;
}
