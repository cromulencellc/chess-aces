#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "connection.hpp"
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
      int fd = listener->accept();
      Connection c(fd);

      try {
        c.service();
      } catch (const OvingtonRuntimeError& ore) {
        LLL.info() << "caught runtime error: " << ore.what();
      }
      listener->dispose(fd);
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

  Connection c(0, 1);

  try {
    while (true) {
      c.service();
    }
  } catch (const OvingtonRuntimeError& ore) {
    LLL.info() << "caught runtime error: " << ore.what();
  }

  return 0;
}
