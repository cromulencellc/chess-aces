#pragma once

#include "error.hpp"

class Listener {
public:
  Listener(uint16_t p);

  int accept();
  void dispose(int fd);

private:
  int sock;
};
