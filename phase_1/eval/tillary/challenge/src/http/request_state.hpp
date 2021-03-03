#pragma once

#include <optional>
#include <vector>

namespace http {
  struct RequestState {
  public:
    bool abort = false;
    bool want_line = false;
    std::optional<size_t> want_bytes;
    bool ready_to_respond = false;

    bool in_valid_state() const;
  };
}

std::ostream& operator<<(std::ostream& o,
                         const http::RequestState& rs);
