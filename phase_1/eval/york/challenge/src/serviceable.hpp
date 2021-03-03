#pragma once

#include "io.hpp"
#include "poller.hpp"

class Serviceable {
public:
  virtual void service(int fd) = 0;
  virtual int input_fd() = 0;

  bool is_finished = false;
};

class FdServiceable : public Serviceable {
public:
  int input_fd() override;

protected:
  FdServiceable(int fd) : io(fd) {};
  FdServiceable(int i_fd, int o_fd) : io(i_fd, o_fd) {};

  Io io;
};
