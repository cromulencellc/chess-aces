#pragma once

#include <map>
#include <ostream>
#include <string>

namespace http {
  using Headers = std::map<std::string, std::string>;
  using Header = Headers::value_type;
}

std::ostream& operator<<(std::ostream& o, const http::Headers& hs);
