#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace http {
  class Target : public std::string {
  public:
    Target(std::string_view& request_line);

    std::filesystem::path path() const;
    std::string_view query() const;

  private:
    Target::const_iterator question_mark() const;
    size_t _question_mark;
  };
}
