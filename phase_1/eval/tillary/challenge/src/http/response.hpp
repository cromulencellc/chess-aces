#pragma once

#include "../logger.hpp"

#include "default_mime_type.hpp"
#include "error.hpp"

#include <memory>


namespace http {
  class Response {
  public:
    virtual ~Response() {}

    virtual void send_headers(int out_fd);

    virtual bool has_countable_body_size() const { return false; }

    virtual int body_size() {
      throw http::BadRequest("called body_size without implementation");
    }

    virtual std::string get_content_type() {
      return content_type;
    }

    virtual int get_status_code() { return 200; }
    virtual std::string get_status_desc() { return "OK"; }

    virtual void stream_body_to(int _out_fd) {
      throw http::BadRequest("called stream_to without implementation");
    }

    std::string content_type = DefaultMimeType;
  };

  using ResponsePtr = std::shared_ptr<Response>;
}
