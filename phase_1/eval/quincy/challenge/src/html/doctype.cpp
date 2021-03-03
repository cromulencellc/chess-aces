#include "doctype.hpp"

#include "error.hpp"
#include "muncher.hpp"

using namespace html;

Doctype::Doctype(ScanIterator& reader) {
  reader.eati("<!doctype");
  Muncher::whitespace(reader, true);
  reader.eati("html");

  if (std::isspace(*reader)) {
    Muncher::whitespace(reader);
    if (reader.can_eati("system")) {
      reader.eati("system");
      Muncher::whitespace(reader, true);
      char quot = *reader;
      reader++;
      reader.eat("about:legacy-compat");
      reader.eat({&quot, 1});
      Muncher::whitespace(reader);
      legacy = true;
    }
  }

  Muncher::whitespace(reader);
  reader.eat(">");

  valid = true;
}
