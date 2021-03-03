#include <cassert>
#include <cstdlib>
#include <iostream>
#include "connection.hpp"
#include "error.hpp"
#include "listener.hpp"
#include "logger.hpp"
int main() {
  std::set_terminate([]() {
    LLL.fatal() << "terminating";
    std::exit(-1);
  });
  std::shared_ptr<Listener> listener = nullptr;
  LLL.info() << "starting poller";
  Poller p;
  if (std::getenv("PORT")) {
    LLL.info() << "starting network";
    int port_big = std::atoi(std::getenv("PORT"));
    uint16_t port = (uint16_t)port_big;
    assert((int)port == port_big);
    listener = std::make_shared<Listener>(port);
    p.register_serviceable(listener);
  }
  try {
    while (true) {
      int evt_count = p.poll();
      LLL.info() << "serviced " << evt_count << " events";
    }
  } catch (const MontagueRuntimeError &mre) {
    LLL.info() << "caught runtime error: " << mre.what();
    LLL.info() << "continuing";
  }
}
