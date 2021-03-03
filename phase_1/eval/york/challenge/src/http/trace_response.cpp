#include "trace_response.hpp"

using namespace http;

void TraceResponse::send_headers(int out_fd) {
  ChunkVec buf;
  buf.reserve(9);
  buf.push_back("HTTP/1.1 ");
  buf.push_back(std::to_string(get_status_code()));
  buf.push_back(" ");
  buf.push_back(get_status_desc());
  buf.push_back("\r\nContent-Type: ");
  buf.push_back(get_content_type());
  buf.push_back("\r\nContent-Length: ");
  buf.push_back(std::to_string(body_size()));
  buf.push_back("\r\nUsing-Resource: ");
  buf.push_back(rsrc_type);
  buf.push_back("\r\n\r\n");

  buf.write(out_fd);
}

void TraceResponse::get_chunks(ChunkVec& buf) {
  size_t reservation = 7;
  reservation += 4 * req.headers.size();
  buf.reserve(reservation);

  if (req.method.has_value()) {
    buf.push_back(req.method.value());
  } else {
    buf.push_back("_NO_METHOD_");
  }
  buf.push_back(" ");
  if (req.target.has_value()) {
    buf.push_back(req.target.value());
  } else {
    buf.push_back("_NO_TARGET_");
  }
  buf.push_back(" ");
  if (req.version.has_value()) {
    buf.push_back(req.version.value());
  } else {
    buf.push_back("_NO_VERSION_");
  }
  buf.push_back("\r\n");

  for (auto hdr : req.headers) {
    buf.push_back(hdr.first);
    buf.push_back(": ");
    buf.push_back(hdr.second);
    buf.push_back("\r\n");
  }

  buf.push_back("\r\n");
}
