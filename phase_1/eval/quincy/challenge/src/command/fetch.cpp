#include "fetch.hpp"

#include "../http/get.hpp"

using namespace command;

using Get = http::Get;

bool Fetch::execute(Io &io, std::string args) {
  LLL.debug() << "getting " + args;
  Get g = {args};

  io.puts("response headers:");
  for (auto h : g.response_headers()) {
    io.puts(h.first + ": " + h.second);
  }
  io.puts("");

  return true;
}
