#pragma once

#include "../serviceable.hpp"

#include "../address.hpp"
#include "../chunk_vec.hpp"
#include "request.hpp"
#include "request_state.hpp"

namespace http {
  class Connection : public FdServiceable {
  public:
    Connection(int client_fd, Address client_address);
    Connection(); // stdio
    virtual ~Connection() {};

    void service(int fd) override;

  private:
    std::string in_buf;
    Request req;
    ChunkVec out_buf;

    bool have_it() const;

    bool have_line() const;
    bool have_bytes() const;

    Address client_addr;
  };
}
