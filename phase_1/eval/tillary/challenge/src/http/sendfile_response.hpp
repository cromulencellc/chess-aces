#pragma once

#include "response.hpp"

namespace http {
  class SendfileResponse : public Response {
  public:
    SendfileResponse(int r) : read_fd(r) {};
    SendfileResponse(int r, std::string ct) : read_fd(r), content_type(ct) {};
    virtual ~SendfileResponse();

    virtual bool has_countable_body_size() const override { return true; }

    int body_size() override;

    std::string get_content_type() override {return content_type;}

    void stream_body_to(int dest_fd) override;
  private:
    int read_fd;
    std::string content_type = DefaultMimeType;
    off_t _body_size = -1;
  };
}
