#include "error_response.hpp"

#include <sstream>

using namespace http;

int ErrorResponse::get_status_code() {
  return runtime_error.status_code();
}

std::string ErrorResponse::get_status_desc() {
  return runtime_error.status_desc();
}

void ErrorResponse::get_chunks(ChunkVec& buf) {
  buf.push_back(std::to_string(runtime_error.status_code()));
  buf.push_back(" ");
  buf.push_back(runtime_error.status_desc());
  buf.push_back("\r\n");
  buf.push_back(runtime_error.inspect());
}
