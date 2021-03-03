#include "sendfile_response.hpp"

#include "../logger.hpp"

#include "mime_types.hpp"

#include <sys/sendfile.h>
#include <unistd.h>

using namespace http;

SendfileResponse::~SendfileResponse() {
  if (read_fd >= 0) ::close(read_fd);
  read_fd = -1;
}

int SendfileResponse::body_size() {
  if (_body_size >= 0) return _body_size;

  off_t cur = lseek(read_fd, 0, SEEK_CUR);
  if (0 > cur) {
    throw http::SystemError();
  }
  off_t begin = lseek(read_fd, 0, SEEK_SET);
  if (0 != begin) {
    throw http::SystemError();
  }
  _body_size = lseek(read_fd, 0, SEEK_END);
  if (0 > _body_size) {
    throw http::SystemError();
  }
  off_t maybe_cur = lseek(read_fd, cur, SEEK_SET);
  if (cur != maybe_cur) {
    throw http::SystemError();
  }

  LLL.debug() << "fd " << std::dec << read_fd
              << " has size " << _body_size;

  return _body_size;
}

void SendfileResponse::stream_body_to(int dest_fd) {
  off_t cur = lseek(read_fd, 0, SEEK_CUR);
  if (0 > cur) {
    throw SystemError();
  }
  off_t begin = lseek(read_fd, 0, SEEK_SET);
  if (0 != begin) {
    throw SystemError();
  }

  ssize_t remain = (ssize_t) body_size();

  while (remain > 0) {
    ssize_t sent = sendfile(dest_fd, read_fd, nullptr, remain);
    remain -= sent;
  }


  off_t maybe_cur = lseek(read_fd, cur, SEEK_SET);
  if (cur != maybe_cur) {
    throw SystemError();
  }
}
