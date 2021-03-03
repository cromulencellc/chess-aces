#pragma once

#include "../resource.hpp"

namespace http {
  namespace resources {
    class Tweets : public Resource {
    public:
      static bool handles(const std::filesystem::path& path);

      Tweets(const Request& rq);
      virtual ~Tweets() {};

      bool allowed_method() override;
      bool exists() override;

      bool acceptable_content_type() override;

      bool allow_post() override;

      void process() override;

      bool did_create() override;
      bool should_redirect() override;
      std::string redirect_location() override;

      ResponsePtr get_response() override;

    protected:
      bool wanted_index = false;
      std::string status_id;

      void try_load();

      bool did_load_status = false;
      bool failed_to_load_status = false;
      std::string text;

      bool is_loadable_status_id() const;
      bool is_index() const;

      ResponsePtr get_index();
      ResponsePtr get_status();
    };
  }
}
