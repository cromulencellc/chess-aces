#include "url.hpp"

#include <charconv>
#include <regex>
#include <sstream>

#include "../logger.hpp"

using namespace http;

// 1: scheme
// 2: host
// 3: port
// 4: target
const std::regex url_parser("(\\w+)://([a-zA-Z0-9.]+)(\\:\\d+)?(/[^#]*)");

Url::Url(std::string url) {
  LLL.debug() << url;
  std::smatch got;
  if (! std::regex_match(url, got, url_parser)) {
    throw InvalidUrlError(url);
  }

  scheme = got[1];
  host = got[2];
  port = got[3];
  target = got[4];

  valid = true;
}

std::string Url::inspect() const {
  std::stringstream buf;

  buf << "Url( scheme=`" << scheme
      << "` host=`" << host
      << "` port=`" << port
      << "` target=`" << target
      << "` )";

  return buf.str();
}

uint16_t Url::port_number(uint16_t def) {
  if (_port_number.has_value()) return _port_number.value();
  if ("" == port) return def;

  uint16_t got;
  auto [ptr, errc] =
    std::from_chars(port.data(), port.data() + port.size(), got);

  if (std::errc() != errc) throw InvalidPortNumberError(port);

  _port_number = got;

  return _port_number.value();
}

std::ostream& operator<<(std::ostream& o, const http::Url& u) {
  o << u.inspect();
  return o;
}
