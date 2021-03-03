#include "html_response.hpp"

#include <regex>

using namespace http;

void HtmlResponse::raw(const std::string& content) {
  chunks.push_back(content);
}

void HtmlResponse::open_tag(const std::string& tag_name,
                            const std::string& attrs) {
  tags.push(tag_name);

  chunks.push_back("<" + tag_name + " " + attrs + ">");
}

std::regex ampersand_finder("&");
std::regex less_than_finder ("<");
std::regex greater_than_finder(">");

void HtmlResponse::text(const std::string& content) {
  std::string escaped_amps =
    std::regex_replace(content, ampersand_finder, "&amp;");
  std::string escaped_lts =
    std::regex_replace(escaped_amps, less_than_finder, "&lt;");
  std::string escaped_gts =
    std::regex_replace(escaped_lts, greater_than_finder, "&gt;");

  chunks.push_back(escaped_gts);
}

void HtmlResponse::close_tag() {
  if (tags.empty()) {
    throw http::RuntimeError("ran out of tags to close");
  }

  chunks.push_back("</" + tags.top() + ">");
  tags.pop();
}

void HtmlResponse::get_chunks(ChunkVec& buf) {
  if (&buf != &chunks) {
    throw http::RuntimeError("asked to get_chunks somewhere weird");
  }
  return;
}
