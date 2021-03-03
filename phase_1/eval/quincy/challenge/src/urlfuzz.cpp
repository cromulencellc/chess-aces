#include "http/url.hpp"
#include "io.hpp"
#include "logger.hpp"

int main() {
  std::set_terminate([](){
                       LLL.fatal() << "terminating";
                       std::exit(-1);
                     });

  Io stdio = Io::stdio();
  std::string candidate = stdio.read_str_delim('\n');

  http::Url u = {candidate};

  std::string got = u.inspect();

  stdio.puts(got);

  return 0;
}
