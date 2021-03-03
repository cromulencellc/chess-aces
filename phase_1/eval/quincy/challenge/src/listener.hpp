#pragma once

#include "io.hpp"

#include <stdint.h>

class Listener {
public:
  Listener(uint16_t listen_port);

  Io accept();

private:
  int sock;
};
