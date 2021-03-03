#pragma once

#include "empty_response.hpp"

namespace http {  
  class BlankResponse : public EmptyResponse {
  public:
    BlankResponse(int status_code, std::string status_desc) :
      code(status_code), desc(status_desc) {};
    virtual ~BlankResponse() {};

    void send_headers(int out_fd) override;
    
    int get_status_code() override { return code; }
    std::string get_status_desc() override { return desc; }

    void stream_body_to(int _out_fd) override { return; }

  private:
    int code;
    std::string desc;
  };

  ResponsePtr make_blank(int status_code, std::string status_desc);
}
