#pragma once

#include "chunky_response.hpp"
#include "request.hpp"
#include "resource.hpp"

#include <typeinfo>

namespace http {
  class TraceResponse : public ChunkyResponse {
  public:
    TraceResponse(const Request& rq, Resource* rs)
      : req(rq),
        rsrc_type(typeid(*rs).name())
    {};
    virtual ~TraceResponse() {};

    void send_headers(int out_fd) override;
    void get_chunks(ChunkVec& buf) override;

    std::string get_content_type() override {
      return "message/http";
    }

  private:
    const Request& req;
    const std::string rsrc_type;
  };
}
