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

Text::Text(ScanIterator& scan) {
  char cur;
  while (!scan.kicked()
         && (cur = *scan)
         && ('<' != cur)) {
    if ('&' == cur) {
      decode_character_reference(scan);
      continue;
    }

    if (std::isspace(cur)) cur = ' ';

    content.push_back(cur);
    scan++;
  }
}

NodePtr Text::parse(ParseIterator& p) const {
  assert_parsing_self(p);
  NodePtr it_me = *p;
  p++;
  return it_me;
}

std::string Text::inspect() const {
  return "Text(contents: " + std::to_string(content.size()) + " bytes)";
}

std::string Text::to_text() const {
  return content;
}


void Text::pretty(PrettyPrinter& printer) const {
  WrapVec words;

  std::string::const_iterator begin = content.cbegin();
  std::string::const_iterator end = content.cend();

  std::string::const_iterator cur_start = begin;
  std::string::const_iterator cur_end = begin;

  enum text_pretty_state { look_start, look_end };

  text_pretty_state state = look_start;

  while (cur_end < end) {
    if (look_start == state) {
      if (std::isspace(*cur_start)) {
        cur_start++;
        cur_end = cur_start;
        continue;
      }

      state = look_end;
    }

    if (!std::isspace(*cur_end)) {
      cur_end++;
      continue;
    }

    const char* start_ptr = &*cur_start;
    cur_end++;
    size_t len = cur_end - cur_start;
    words.push_back({start_ptr, len});
    state = look_start;
    cur_start = cur_end;
    cur_end = cur_start;
  }

  if (look_end == state) {
    const char* start_ptr = &*cur_start;
    size_t len = cur_end - cur_start;
    words.push_back({start_ptr, len});
  }

  if (0 == words.size()) return;

  printer.wrap(words);
}

const std::regex decimal_reference_parser("\\d+");
const std::regex hex_reference_parser("[0-9a-fA-F]+");

void Text::decode_character_reference(ScanIterator& scan) {
  scan.eat(";");
  std::string ref;
  char cur;
  while ((cur = *scan)
         && (';' != cur)) {
    ref.push_back(cur);
    scan++;
    scan.expect_not_kicked();
  }

  std::smatch got;
  if (std::regex_match(ref, got, decimal_reference_parser)) {
    char referent;
    auto [ptr, errc] = std::from_chars(&*ref.cbegin(),
                                       &*ref.cend(),
                                       referent);
    if (std::errc() != errc) {
      content.push_back('&');
      content.append(ref);
      content.push_back(';');
    } else {
      content.push_back(referent);
    }
    return;
  }

  if (std::regex_match(ref, got, hex_reference_parser)) {
    char referent;
    auto [ptr, errc] = std::from_chars(&*ref.cbegin(),
                                       &*ref.cend(),
                                       referent,
                                       16);
    if (std::errc() != errc) {
      content.push_back('&');
      content.append(ref);
      content.push_back(';');
    } else {
      content.push_back(referent);
    }
    return;
  }

  std::optional<std::string> maybe_substitution =
    substitute_entity(ref);

  if (maybe_substitution) {
    content.append(maybe_substitution.value());
    return;
  }

  content.push_back('&');
  content.append(ref);
  content.push_back(';');
}
