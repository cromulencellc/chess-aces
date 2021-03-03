#pragma once
#include "error.hpp"
#include "request.hpp"
#include "uuid.hpp"
#include "mtl.hpp"
#include "mtl/template.hpp"
#include "response/headers.hpp"
#include "response/status.hpp"
#include <ostream>
#include <string>
#include <sys/uio.h>
#include <vector>
class Response {
public:
  Response(Uuid tx_id, Request rq);
  response::Status status;
  response::Headers headers;
  std::vector<struct iovec> body;
  std::string serialize();
  void serialize_to(int fd);

private:
  Uuid transaction_id;
  Request request;
  mtl::Template tpl;
  mtl::SlugBag got;
  std::shared_ptr<mtl::Context> ctx;
};
namespace response {
class WritevError : public MontagueSystemError {};
class ShortWritevError : public MontagueRuntimeError {
public:
  ShortWritevError(size_t expected, ssize_t got);
};
}
std::ostream &operator<<(std::ostream &o, Response &rs);
