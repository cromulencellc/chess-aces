#include "muncher.hpp"

#include "error.hpp"

using namespace html;

void Muncher::whitespace(ScanIterator& s, bool mandatory) {
  if (mandatory) {
    if (s.kicked()) throw UnexpectedlyKicked("whitespace");
    if (!std::isspace(*s)) throw NeededWS(*s);
  }

  while ((!s.kicked()) && std::isspace(*s)) { s++; }
}

void end_comment(ScanIterator& s) {
  while (!s.kicked()) {
    if (s.has(3) &&
        ('-' == *s) &&
        ('-' == *(s+1)) &&
        ('>' == *(s+2))) {
      s += 3;
      return;
    }
    s += 1;
  }
}

void Muncher::comments_and_whitespace(ScanIterator& s) {
  while (!s.kicked()) {
    // not in comment yet
    if (s.can_eat("<!--")) {
      // found a comment
      s.eat("<!--");
      end_comment(s);
      continue;
    }

    if (! std::isspace(*s)) return;

    s += 1;
  }
}

void Muncher::comment(ScanIterator& s) {
  s.eat("<!--");
  while (! s.kicked()) {
    if (s.can_eat("-->")) {
      s.eat("-->");
      return;
    }

    s++;
  }

  throw UnexpectedlyKicked("-->");
}
