#pragma once

#include "oqtopus/environment.hpp"

class Oqtopus {
public:
  void consume(int in_fd, int out_fd);

  bool want_data_load = false;
  bool want_disconnect = false;

private:
  oqtopus::Environment env = oqtopus::Environment();
};
