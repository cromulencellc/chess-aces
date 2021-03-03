#include "response.hpp"

#include "../chunk_vec.hpp"

using namespace http;

void Response::send_headers(int out_fd) {
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
  buf.push_back("\r\n\r\n");

  buf.write(out_fd);
}
