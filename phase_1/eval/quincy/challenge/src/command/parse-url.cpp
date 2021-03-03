#include "parse-url.hpp"

#include "../http/url.hpp"

using namespace command;

bool ParseUrl::execute(Io &io, std::string args) {
  LLL.debug() << "trying to parse `" << args << "`";

  http::Url u;
  try {
    u = {args};
  } catch (const RuntimeError& e) {
    LLL.debug() << "parsing url " << args << " and got " << e.inspect();
    io.write_str("failed to parse url `" + args + "`");
    return true;
  }

  if (! u.valid ) {
    LLL.error() << "invalid url got through somehow??";
    return false;
  }

  std::string got = u.inspect();

  io.puts(got);

  return true;
}
