#pragma once

#include "response.hpp"

namespace http {
  class RedirectResponse : public Response {
  public:
    RedirectResponse(int status_code,
                     std::string status_desc,
                     std::string destination) :
      code(status_code), desc(status_desc), location(destination) {};
    virtual ~RedirectResponse() {};

    void send_headers(int out_fd) override;

    bool has_countable_body_size() const override { return true; }
    int body_size() override { return 0; }

    int get_status_code() override { return code; }
    std::string get_status_desc() override { return desc; }

    void stream_body_to(int _out_fd) override { return; }
  private:
    int code;
    std::string desc;
    std::string location;
  };

  ResponsePtr make_redirect(int status_code,
                            std::string status_desc,
                            std::string location);
}
