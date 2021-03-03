#pragma once

#include "error.hpp"

#include <optional>
#include <string>

namespace html {
  std::optional<std::string> substitute_entity(std::string candidate);
}
