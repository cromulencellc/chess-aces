#include "target.hpp"

#include "error.hpp"
#include "../logger.hpp"

const size_t max_target_length = 512;

using namespace http;

Target::Target(std::string_view& request_line) : std::string(request_line) {
  std::string::size_type first_space = find_first_of(' ');

  if (0 == first_space) {
    throw BadRequest("couldn't parse target (too many spaces before)");
  }

  if (std::string_view::npos == first_space) {
    throw BadRequest("couldn't parse target (no space afterwards)");
  }

  request_line.remove_prefix(first_space + 1);

  resize(first_space);

  size_t cur = 0;
  size_t end = size();

  if (size() > max_target_length) {
    throw URITooLong();
  }

  while (cur < end) {
    if ('?' == at(cur)) {
      break;
    }
    cur++;
  }

  _question_mark = cur;
}

Target::const_iterator Target::question_mark() const {
  return cbegin() + _question_mark;
}

std::filesystem::path Target::path() const {
  return {cbegin(), question_mark()};
}

std::string_view Target::query() const {
  if (cend() == question_mark()) return {};

  size_type len = cend() - question_mark();
  const char* start = &*question_mark();
  return {start, len};
}
