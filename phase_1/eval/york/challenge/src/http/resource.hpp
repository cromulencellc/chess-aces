#pragma once

#include <memory>
#include <string>

#include "method.hpp"
#include "request.hpp"
#include "response.hpp"

namespace http {
  class Resource {
  public:
    static bool handles(const std::filesystem::path& _path) { return false; }

    Resource(const Request& rq) : req(rq) {};
    virtual ~Resource() {};

    virtual bool allowed_method() { return false; }
    virtual bool known_method();
    virtual bool acceptable_content_type();

    virtual bool forbidden() { return false; }
    virtual bool exists() { return false; }

    virtual bool allow_post() { return false; }

    virtual void process() {
      throw http::RuntimeError("don't know how to process");
    }

    virtual bool not_committed() {
      return false;
    }
    virtual bool has_response_content() {
      return false;
    }
    virtual bool did_create() {
      return false;
    }
    virtual bool should_redirect() {
      return false;
    }
    virtual std::string redirect_location() {
      throw http::RuntimeError("wanted redirect_location");
    }

    virtual ResponsePtr get_response() {
      throw http::RuntimeError("don't know how to get_response");
    }
    virtual ResponsePtr head_response() { return get_response(); }
    virtual ResponsePtr post_response() {
      throw http::RuntimeError("don't know how to post_response");
    }
    virtual ResponsePtr delete_response() {
      throw http::RuntimeError("don't know how to delete_response");
    }
    virtual ResponsePtr put_response() {
      throw http::RuntimeError("don't know how to put_response");
    }
    virtual ResponsePtr trace_response();

  protected:
    const Request& req;
  };

  using ResourcePtr = std::unique_ptr<Resource>;
}
