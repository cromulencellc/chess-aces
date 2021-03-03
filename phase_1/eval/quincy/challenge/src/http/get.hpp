#pragma once

#include <optional>
#include <string>

#include "headers.hpp"
#include "../io.hpp"
#include "url.hpp"

namespace http {
  class Get {
  public:
    Get(std::string u) : url(u) {};

    void send();

    int status() { return _status; }

    const Headers& response_headers();
    std::optional<std::string> body();

  private:
    Url url;

    int _status = -1;
    std::string _status_description;
    std::string _body;

    const Headers& request_headers();

    Headers _request_headers;
    Headers _response_headers;

    void read_status_line(Io& server);
    void read_headers(Io& server);
    void read_response_by_content_length(Io& server);
  };
}
