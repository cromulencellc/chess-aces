#include "blank_response.hpp"

#include "../chunk_vec.hpp"

using namespace http;

void BlankResponse::send_headers(int out_fd) {
  ChunkVec buf;
  buf.reserve(6);
  buf.push_back("HTTP/1.1 ");
  buf.push_back(std::to_string(code));
  buf.push_back(" ");
  buf.push_back(desc);
  buf.push_back("\r\nServer: tillary\r\n\r\n");
  buf.push_back("\r\n\r\n");

  buf.write(out_fd);
}

ResponsePtr http::make_blank(int status_code, std::string status_desc) {
  return std::make_shared
    <BlankResponse>(status_code, status_desc);
}
