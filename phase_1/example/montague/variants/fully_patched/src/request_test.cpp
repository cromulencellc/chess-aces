#include <cassert>
#include <cstdlib>
#include <iostream>
#include "address.hpp"
#include "error.hpp"
#include "logger.hpp"
#include "request.hpp"
int main() {
  std::set_terminate([]() {
    LLL.fatal() << "terminating";
    std::exit(-1);
  });
  address::Address4 lh = {.sin_family = AF_INET,
                          .sin_port = 12345,
                          .sin_addr = {.s_addr = 0x7f000001}};
  try {
    Request rq(0, *(Address *)&lh);
    LLL.info() << rq;
  } catch (const MontagueSystemError &mse) {
    LLL.error() << "System Error " << mse.code() << " re: " << mse.what();
  } catch (const MontagueRuntimeError &mre) {
    LLL.error() << "Runtime Error " << mre.what();
  }
}
