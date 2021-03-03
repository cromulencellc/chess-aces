#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "ui.hpp"
#include "error.hpp"
#include "listener.hpp"
#include "logger.hpp"
#include "testbed.hpp"

int network() {
  std::unique_ptr<Listener> listener = nullptr;

  LLL.info() << "starting network";

    int port_big = std::atoi(std::getenv("PORT"));
    uint16_t port = (uint16_t) port_big;
    assert((int) port == port_big);

    listener = std::make_unique<Listener>(port);

    while (true) {
      Io cli = listener->accept();
      Ui ui(cli);

      try {
        ui.service();
      } catch (const Error& e) {
        LLL.error() << e.inspect();
      }
    }

    return 0;
}


int main() {
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif

  std::set_terminate([](){
                       LLL.fatal() << "terminating";
                       std::exit(-1);
                     });

  if (std::getenv("PORT")) {
    return network();
  }

  // stdin and stdout
  Io stdio = Io::stdio();
  Ui ui(stdio);

  try {
    ui.service();
  } catch (const Error& e) {
    LLL.error() << e.inspect();
  }

  return 0;
}
