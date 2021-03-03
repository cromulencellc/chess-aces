#pragma once

#include <iostream>

class Net {
 public:
  Net(int p);

  std::istream* get_in();
  std::ostream* get_out();

  int port;
  int client_fd;
};
