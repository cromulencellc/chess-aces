#include "get.hpp"

#include <charconv>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <regex>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../address.hpp"
#include "../hton.hpp"
#include "../io.hpp"
#include "../logger.hpp"

#include "error.hpp"

#define throw_nonzero(val) {if (0 != (val)) throw http::SystemError();}

using namespace http;
using namespace address;

const std::regex header_parser("([a-zA-Z\\-]+):\\s+([^\\r]+)\\r\\n");

void Get::send() {
  if (-1 != status()) return;

  LLL.info() << "sending GET for " << url;
  struct addrinfo hints = {
                           .ai_family = AF_UNSPEC,
                           .ai_socktype = SOCK_STREAM,
                           .ai_protocol = IPPROTO_TCP
  };

  struct addrinfo* result;

  int info_status = getaddrinfo(url.host.c_str(),
                                url.port.c_str() + 1,
                                &hints, &result);
  if (0 != info_status) throw AddrInfoError(info_status);

  LLL.debug() << "got addresses:";
  for (struct addrinfo *p = result; p != nullptr; p = p->ai_next) {
    if (AF_INET == p->ai_family) {
      LLL.debug() << *(Address4*)(p->ai_addr);
    } else if (AF_INET6 == p->ai_family) {
      LLL.debug() << *(Address6*)(p->ai_addr);
    }
  }
  LLL.debug() << "using the first one";

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sock_fd) throw http::SystemError();

  //  result->ai_addr->

  int connect_status = connect(sock_fd, result->ai_addr, result->ai_addrlen);
  freeaddrinfo(result);

  throw_nonzero(connect_status);

  Io server = {sock_fd};

  std::stringstream req_builder;
  req_builder << "GET " << url.target << " HTTP/1.1" << "\r\n";
  req_builder << request_headers();
  req_builder << "\r\n";

  server.write_str(req_builder.str());

  read_status_line(server);
  read_headers(server);

  try {
    _request_headers.at("Content-Length");
  }
  catch (const std::out_of_range& _oor) {
    throw http::RuntimeError("couldn't read body without Content-Length");
  }

  read_response_by_content_length(server);
}

const Headers& Get::request_headers() {
  if (0 != _request_headers.size()) return _request_headers;
  _request_headers["Host"] = url.host;

  return _request_headers;
}

const Headers& Get::response_headers() {
  if (-1 == status()) send();
  return _response_headers;
}

std::optional<std::string> Get::body() {
  if (-1 == status()) send();

  return std::make_optional<std::string>(_body);
}

void Get::read_status_line(Io& server) {
  std::string _http_version = server.read_str_delim(' ');
  std::string status_line = server.read_str_delim('\n');
  std::string last_two = status_line.substr(status_line.size() - 2);
  if ("\r\n" != last_two) {
    throw http::RuntimeError("Couldn't find crlf at end of: " + status_line);
  }

  int got;
  auto [ptr, errc] = std::from_chars(status_line.data(),
                                     status_line.data() + status_line.size(),
                                     got);

  if (std::errc() != errc) {
    throw http::RuntimeError("Couldn't find status integer in: " + status_line);
  }

  _status = got;
  _status_description = status_line.substr((ptr + 1) - status_line.data());
}

void Get::read_headers(Io& server) {
  std::string header_line = server.read_str_delim('\n');
  if ("\r\n" == header_line) return;

  std::smatch got;
  if (! std::regex_match(header_line, got, header_parser)) {
    throw http::RuntimeError("Couldn't parse header: " + header_line);
  }

  std::string key = got[1];
  std::string value = got[2];
  _request_headers.insert_or_assign(key, value);

  return read_headers(server);
}

void Get::read_response_by_content_length(Io& server) {
  std::string& response_len_contents = _request_headers.at("Content-Length");
  uint32_t response_len;
  auto [ptr, errc] = std::from_chars(&*response_len_contents.cbegin(),
                                     &*response_len_contents.cend(),
                                     response_len);
  if (std::errc() != errc) {
    throw http::RuntimeError("Couldn't parse content-length in: " +
                             response_len_contents);
  }

  _body = server.read_str_len(response_len);
}
