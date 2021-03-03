#include "redirect_response.hpp"

#include "../chunk_vec.hpp"

using namespace http;

void RedirectResponse::send_headers(int out_fd) {
  ChunkVec buf;
  buf.reserve(7);
  buf.push_back("HTTP/1.1 ");
  buf.push_back(std::to_string(code));
  buf.push_back(" ");
  buf.push_back(desc);
  buf.push_back("\r\nLocation: ");
  buf.push_back(location);
  buf.push_back("\r\nContent-length: 0");
  buf.push_back("\r\n\r\n");

  buf.write(out_fd);
}

ResponsePtr http::make_redirect(int status_code, std::string status_desc,
                                std::string location) {
    return std::make_shared
      <RedirectResponse>(status_code, status_desc,
      location);
  }
