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

NodePtr Null::parse(ParseIterator& p) const {
  assert_parsing_self(p);
  NodePtr it_me = *p;
  p++;
  return it_me;
}
