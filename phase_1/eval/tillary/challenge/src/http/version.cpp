#include "version.hpp"

#include "error.hpp"

using namespace http;

Version::Version(std::string_view& request_line) : std::string(request_line) {
  request_line.remove_prefix(size());
}
