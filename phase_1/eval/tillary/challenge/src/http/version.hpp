#pragma once

#include <string>
#include <string_view>

namespace http {
  class Version : public std::string {
  public:
    Version(std::string_view& request_line);
  };
}
