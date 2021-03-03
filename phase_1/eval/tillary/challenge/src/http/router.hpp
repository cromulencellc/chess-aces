#pragma once

#include "resource.hpp"

namespace http {
  class Router {
  public:
    static ResourcePtr route(const Request& req);
  };
}
