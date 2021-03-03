#pragma once

#include "static.hpp"

namespace http {
  namespace resources {
    class Favicon : public Static {
    public:
      static bool handles(const std::filesystem::path& path);

      Favicon(const Request& rq) : Static(rq) {};
      virtual ~Favicon() {};

      void validate_path(std::filesystem::path& p) override;
    };
  }
}
