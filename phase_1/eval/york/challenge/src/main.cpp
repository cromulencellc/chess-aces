#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "error.hpp"
#include "http/connection.hpp"
#include "io.hpp"
#include "listener.hpp"
#include "logger.hpp"
#include "poller.hpp"
#include "testbed.hpp"

void network() {
  std::shared_ptr<Listener> listener = nullptr;

  LLL.info() << "starting network";

    int port_big = std::atoi(std::getenv("PORT"));
    uint16_t port = (uint16_t) port_big;
    assert((int) port == port_big);

    listener = std::make_shared<Listener>(port);

    Poller::register_serviceable(listener);
}

void connect_stdin() {
  std::shared_ptr<http::Connection> conn = std::make_shared<http::Connection>();

  Poller::register_serviceable(conn);
}

int main() {
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif

  std::set_terminate([](){
                       LLL.fatal() << "terminating";
                       std::exit(-1);
                     });

  Poller p;

  if (std::getenv("PORT")) {
    network();
  } else {
    connect_stdin();
  }

  try {
    while (true) {
      p.poll();
    }
  } catch (const Error& e) {
    LLL.error() << e.inspect();
  }

  return 0;
}
