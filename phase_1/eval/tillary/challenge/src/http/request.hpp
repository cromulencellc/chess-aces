#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "error.hpp"
#include "method.hpp"
#include "request_state.hpp"
#include "target.hpp"
#include "version.hpp"

namespace http {
  class Request {
  public:
    void consume(std::string_view element);

    RequestState want = {.want_line = true};

    Method::Name method_name() const {
      if (! method.has_value()) {
        throw http::RuntimeError("tried to get method name of invalid request");
      }
      return method.value_or(Method()).name;
    }

    std::filesystem::path path() const {
      if (! target.has_value()) {
        throw http::RuntimeError("tried to get path of invalid request");
      }
      return target->path();
    }

    std::optional<Method> method;
    std::optional<Target> target;
    std::optional<Version> version;

    std::map<std::string, std::string> headers;

    std::unique_ptr<std::string> body; // nullable

  private:
    bool did_parse_first_line = false;
    bool did_parse_headers = false;
    bool did_consume_whole_body = false;

    void consume_first_line(std::string_view element);
    void consume_header_line(std::string_view element);
    void consume_body_chunk(std::string_view element);

    void determine_body_size();
  };
}
