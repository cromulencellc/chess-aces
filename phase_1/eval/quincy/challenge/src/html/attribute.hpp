#pragma once

#include <ostream>
#include <string>

namespace html {
  class Attribute {
  public:
    Attribute(std::string base_str,
              ssize_t& scan,
              const ssize_t& end_of_attrs);

    bool valid;

    char quot;

    ssize_t start;
    ssize_t eq;
    ssize_t end;

    std::string to_text(const std::string& attribute_text) const;

    std::string inspect() const;
  };
}

std::ostream& operator<<(std::ostream& o, const html::Attribute& a);
