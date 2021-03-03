#include "request.hpp"

#include "error.hpp"

#include "../logger.hpp"

#include <cstdio>
#include <type_traits>

using namespace http;

const size_t max_request_header_len = 512;
const size_t max_request_header_count = 256;

void Request::consume(std::string_view element) {
  if (! want.in_valid_state()) {
    throw http::RuntimeError("invalid request state");
  }
  if (!did_parse_first_line) {
    consume_first_line(element);
  } else if (!did_parse_headers) {
    consume_header_line(element);
  } else if (!did_consume_whole_body) {
    consume_body_chunk(element);
  }

  LLL.info() << want;
}

void Request::consume_first_line(std::string_view element) {
  method = {element};
  target = {element};
  version = {element};

  if (0 != element.size()) {
    throw BadRequest("weird stuff after the first line");
  }

  want = {.want_line = true};
  did_parse_first_line = true;
}

void Request::consume_header_line(std::string_view element) {
  if (0 == element.size()) {
    did_parse_headers = true;
    determine_body_size();
    return;
  }

  if (element.size() > max_request_header_len) {
    throw RequestHeaderFieldsTooLarge();
  }

  std::string_view::size_type colon = element.find_first_of(':');

  if (0 == colon) {
    throw BadRequest("couldn't parse header (colon in leading position)");
  }
  if (std::string_view::npos == colon) {
    throw BadRequest("couldn't parse header (no colon)");
  }

  std::string header_name {element.substr(0, colon)};

  element.remove_prefix(colon + 1);

  std::string_view::size_type first_part_of_value =
    element.find_first_not_of(" ");
  element.remove_prefix(first_part_of_value);

  std::string_view::size_type last_part_of_value =
    element.find_first_not_of(" ");
  element.remove_suffix(last_part_of_value);

  std::string header_value {element};

  std::string& cur = headers[header_name];

  if (headers.size() > max_request_header_count) {
    throw RequestHeaderFieldsTooLarge();
  }

  if (0 != cur.size()) { // it's not new
    cur.append(",");
  }

  cur.append(header_value);

  LLL.info() << header_name << ": " << header_value;
}

void Request::consume_body_chunk(std::string_view element) {
  if (! want.want_bytes.has_value()) {
    throw http::RuntimeError("consume_body_chunk but didn't want bulk bytes");
  }

  if (element.size() > want.want_bytes.value()) {
    throw http::RuntimeError("wanted fewer bytes than given");
  }

  if (nullptr == body) body = std::make_unique<std::string>();

  if (element.size() == 0) {
    return;
  }

  body->append(element);

  size_t new_desire = want.want_bytes.value() - element.size();

  if (0 == new_desire) {
    did_consume_whole_body = true;
    want = {.ready_to_respond = true};
    return;
  }

  want = {.want_bytes = new_desire};
}

void Request::determine_body_size() {
  std::string got_content_length = headers["Content-Length"];
  if (0 != got_content_length.size()) {
    // has body of known size
    static_assert(std::is_same<size_t, unsigned long>::value);
    size_t desire;
    int got = std::sscanf(got_content_length.c_str(), "%lu", &desire);

    if (1 != got) {
      throw BadRequest("tried and failed to parse Content-Length: `" +
                         got_content_length +
                         "`");
    }

    if (0 == desire) {
      want = {.ready_to_respond = true};
      return;
    }
    want = {.want_bytes = desire};

    return;
  }

  headers.erase("Content-Length");

  std::string got_transfer_encoding = headers["Transfer-Encoding"];
  if (0 != got_transfer_encoding.size()) {
    throw LengthRequired();
  }

  headers.erase("Transfer-Encoding");

  want = {.ready_to_respond = true};
  return;
}
