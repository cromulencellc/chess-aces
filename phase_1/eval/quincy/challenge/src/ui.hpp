#pragma once

#include "io.hpp"

class Ui {
public:
  Ui(Io& fds) : io(fds) {};

  void service();

private:
  Io& io;
};
