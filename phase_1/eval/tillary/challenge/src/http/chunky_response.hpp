#pragma once

#include "../chunk_vec.hpp"

#include "response.hpp"

namespace http {
  class ChunkyResponse : public Response {
  public:
    ChunkyResponse() {};
    virtual ~ChunkyResponse() {};

    bool has_countable_body_size() const override { return true; }
    int body_size() override;

    virtual void get_chunks(ChunkVec& buf) = 0;

    void stream_body_to(int out_fd) override;

  protected:
    ChunkVec chunks = {};
    bool did_get_chunks = false;
  };
}
