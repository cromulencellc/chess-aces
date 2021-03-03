#pragma once

#include "connection_state.hpp"
#include "oqtopus.hpp"

class Connection {
public:
  Connection(int f) : in_fd(f), out_fd(f) {};
  Connection(int i, int o) : in_fd(i), out_fd(o) {};

  void service();
private:
  ConnectionState state = ConnectionState::oqtopus;
  Oqtopus oqto;

  int in_fd, out_fd;
};
