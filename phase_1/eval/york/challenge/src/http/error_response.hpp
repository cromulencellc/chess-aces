#pragma once

#include "error.hpp"

#include "chunky_response.hpp"

#include "../chunk_vec.hpp"

namespace http {
  class ErrorResponse : public ChunkyResponse {
  public:
    ErrorResponse(const http::RuntimeError& re) : runtime_error(re) {};
    virtual ~ErrorResponse() {}

    std::string get_content_type() override {
      return "text/plain";
    }

    int get_status_code() override;
    std::string get_status_desc() override;

    void get_chunks(ChunkVec& buf) override;

  private:
    const http::RuntimeError& runtime_error;
  };
}
