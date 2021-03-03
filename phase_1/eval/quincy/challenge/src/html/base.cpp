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

NodePtr Base::parse_document(ScanIterator& scan) {
  NodeVec scanned;

  Muncher::comments_and_whitespace(scan);

  while (!scan.kicked()) {
    if (scan.can_eati("</")) {
      scanned.push_back(mks(CloseTag(scan)));
    } else if (scan.can_eati("<!--")) {
      Muncher::comment(scan);
    } else if (scan.can_eati("<")) {
      scanned.push_back(mks(Tag(scan)));
    } else {
      scanned.push_back(mks(Text(scan)));
    }

  }

  LLL.debug() << scanned.inspect();

  ParseIterator parse = {scanned.cbegin(), scanned.cend()};

  return mks(RootElement(parse));
}

void Base::assert_parsing_self(ParseIterator& p) const {
  if (p.kicked()) throw UnexpectedlyKicked(inspect());
  if (this == &**p) return;

  throw DidntParseSelf(inspect(), (*p)->inspect());
}

bool Base::closes(const std::string _candidate_name) const {
  return false;
}
