#include "request_state.hpp"

#include <ostream>

using namespace http;

bool RequestState::in_valid_state() const {
  if (abort) return true; // yolo disconnect always works

  if (want_line) {
    if (want_bytes) return false;
    if (ready_to_respond) return false;

    return true;
  }

  if (want_bytes) {
    if (0 == want_bytes) return false;
    if (ready_to_respond) return false;

    return true;
  }

  if (ready_to_respond) {
    return true;
  }

  return false;
}

std::ostream& operator<<(std::ostream& o,
                         const http::RequestState& rs) {
  o << "http:RequestState(";
  if (!rs.in_valid_state()) {
    o << "INVALID ";
  }

  o << "abort(" << rs.abort << ") ";
  o << "want_line(" << rs.want_line << ") ";

  if (rs.want_bytes.has_value()) {
    o << "want_bytes(" << rs.want_bytes.value() << ") ";
  } else {
    o << "want_bytes(no) ";
  }

  o << "ready_to_respond(" << rs.ready_to_respond << ")";

  o << ")";

  return o;
}
