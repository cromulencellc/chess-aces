#pragma once

#include "response.hpp"

namespace http {
  class EmptyResponse : public Response {
  public:
    EmptyResponse() {};
    virtual ~EmptyResponse() {};

    bool has_countable_body_size() const override { return true; }
    int body_size() override { return 0; }

    void stream_body_to(int _out_fd) override { return; }
  };
}
