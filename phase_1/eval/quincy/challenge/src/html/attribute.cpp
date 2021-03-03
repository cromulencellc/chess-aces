#include "attribute.hpp"

#include "error.hpp"
#include "../logger.hpp"

#include <sstream>

using namespace html;

Attribute::Attribute(std::string base_str,
                     ssize_t& scan,
                     const ssize_t& end_of_attrs) {
  const char* base = base_str.data();
  while (std::isspace(base[scan])) scan++;

  start = scan;
  while ('=' != base[scan]) {
    scan++;
    if (scan > end_of_attrs) {
      valid = false;
      return;
    }
  }

  eq = scan;
  scan++;
  if (('\'' != base[scan]) && ('"' != base[scan])) {
    throw DidntGet("single- or double-quote", std::string(base[scan], 1));
  }
  quot = base[scan];

  scan++;

  while (quot != base[scan]) {
    scan++;
#ifdef PATCH_STOP_PAST_END_OF_ATTRS
    if (scan > end_of_attrs) {
      valid = false;
      return;
    }
#endif
  }

  end = scan++;

  valid = true;
}

std::string Attribute::to_text(const std::string& attribute_text ) const {
  std::stringstream ret;

  for (ssize_t cur = start; cur < eq; cur++) {
    ret << attribute_text[cur];
  }
  ret << "=" << quot;
  for (ssize_t cur = eq + 2; cur < end; cur++) {
    ret << attribute_text[cur];
  }
  ret << quot;

  return ret.str();
}

std::string Attribute::inspect() const {
  std::stringstream ret;

  ret << "Attribute(valid: " << (valid ? "true" : "false") <<
    " quot: `" << quot << "`" <<
    " start: " << start <<
    " eq: " << eq <<
    " end: " << end << ")";

  return ret.str();
}

std::ostream& operator<<(std::ostream& o, const Attribute& a) {
  o << a.inspect();
  return o;
}
