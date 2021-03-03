#include "chunky_response.hpp"

using namespace http;

int ChunkyResponse::body_size() {
  if (! did_get_chunks) get_chunks(chunks);
  did_get_chunks = true;
  return chunks.content_length();
}

void ChunkyResponse::stream_body_to(int out_fd) {
  if (! did_get_chunks) get_chunks(chunks);
  did_get_chunks = true;
  chunks.write(out_fd);
}
