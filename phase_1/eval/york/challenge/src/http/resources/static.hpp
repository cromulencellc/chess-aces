#pragma once

#include "../resource.hpp"

namespace http {
  namespace resources {
    class Static : public Resource {
    public:
      static bool handles(const std::filesystem::path& path);

      Static(const Request& rq);
      virtual ~Static();

      bool allowed_method() override;
      bool forbidden() override;
      bool exists() override;

      bool acceptable_content_type() override;

      ResponsePtr get_response() override;

      virtual void validate_path(std::filesystem::path& p);

    protected:

      std::filesystem::path path_inside_static;
      int read_fd = -2; // should get set to either an fd or -1 in ctor
      int read_error; // don't care unless read_fd <= 0

      bool did_try_open = false;

      void try_open();
    };
  }
}
