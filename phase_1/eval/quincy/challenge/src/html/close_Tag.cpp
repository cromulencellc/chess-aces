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

CloseTag::CloseTag(ScanIterator& scan) {
  scan.eat("</");
  name = scan.scan_to('>');
}

bool CloseTag::closes(const std::string candidate_name) const {
  return (candidate_name == name);
}

NodePtr CloseTag::parse(ParseIterator& p) const {
  throw CantReallyParse(inspect());
}

std::string CloseTag::inspect() const {
  return "CloseTag(name: `"
    + name
    + "`)";
}

std::string CloseTag::to_text() const {
  return "</" + name + ">";
}

void CloseTag::pretty(PrettyPrinter& printer) const {
  printer.wrap({"</",name,">"});
}
