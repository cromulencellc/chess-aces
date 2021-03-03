#pragma once

#include "../error.hpp"

namespace http {
  class RuntimeError : public ::RuntimeError {
  public:
    RuntimeError() : ::RuntimeError("HTTP Runtime Error") {};
    error_string_constructor(RuntimeError, ::RuntimeError);

    virtual int status_code() const { return 500; }
    virtual const std::string status_desc() const {
      return "Internal Server Error";
    }
  };

  class SystemError : public http::RuntimeError,
                      public ::SystemError {
    std::string inspect() const override {
      return ::SystemError::inspect();
    }
  };

#define _statcode(c) int status_code() const override { return c; }
#define _statdesc(d) const std::string status_desc() const override {\
    return d;\
  }

#define _errklass(klass_name, parent, name, code) \
  class klass_name :  public parent {             \
  public:                                         \
  klass_name() : klass_name(name) {};             \
  error_string_constructor(klass_name, parent);   \
  _statcode(code);                                \
  _statdesc(name);                                \
  };

  _errklass(BadRequest, http::RuntimeError, "Bad Request", 400);
  _errklass(Forbidden, BadRequest, "Forbidden", 403);
  _errklass(NotFound, BadRequest, "Not Found", 404);
  _errklass(MethodNotAllowed, BadRequest, "Method Not Allowed", 405);
  _errklass(LengthRequired, BadRequest, "Length Required", 411);
  _errklass(PayloadTooLarge, BadRequest, "Payload Too Large", 413);
  _errklass(URITooLong, BadRequest, "URI Too Long", 414);
  _errklass(UnsupportedMediaType, BadRequest,
            "Unsupported Media Type", 415);
  _errklass(RequestHeaderFieldsTooLarge, BadRequest,
            "Request Header Fields Too Large", 431);

  _errklass(NotImplemented, http::RuntimeError, "Not Implemented", 501);
}
