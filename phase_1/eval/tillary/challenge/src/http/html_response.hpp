#pragma once

#include <stack>

#include "chunky_response.hpp"

namespace http {
  class HtmlResponse : public ChunkyResponse {
  public:
    HtmlResponse() {};
    virtual ~HtmlResponse() {};

    void raw(const std::string& content);
    void open_tag(const std::string& tag_name,
                  const std::string& attrs = "");
    void text(const std::string& content);
    void close_tag();

    void get_chunks(ChunkVec& buf) override;

    std::string get_content_type() override {
      return "text/html";
    }

  private:
    std::stack<std::string> tags;
  };
}
