#include "parse-doc.hpp"

#include "../html/document.hpp"
#include "../http/get.hpp"
#include "../http/url.hpp"

#include "../uuid.hpp"

using namespace command;

bool ParseDoc::execute(Io& io, std::string args) {
  LLL.debug() << "getting and parsing " << args;

  http::Get g(args);
  g.send();

  LLL.debug() << "status " << g.status();

  std::optional<std::string> maybe_body = g.body();

  if (! maybe_body) {
    io.puts("no body: ");
    io.puts(std::to_string(g.status()));

    return true;
  }

  Uuid u = {};

  io.puts(to_string(u));

  std::string body = maybe_body.value();

  LLL.debug() << body;

  html::Document doc(body);

  doc.pretty(io);

  io.puts(to_string(u));

  return true;
}
