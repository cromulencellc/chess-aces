#pragma once

#include "empty_response.hpp"

namespace http {
  class NoContentResponse : public EmptyResponse {
  public:
    NoContentResponse() {};
    virtual ~NoContentResponse() {};

    int get_status_code() override { return 204; }
    std::string get_status_desc() override { return "No Content"; }
  };
}
