#pragma once

#include <iostream>

#include "error.hpp"
#include "serviceable.hpp"

class Listener : public Serviceable {
public:
  Listener(uint16_t port_number);
  virtual ~Listener();

  void service(int fd) override;
  int input_fd() override;

private:
  uint16_t port;
  int sock;
};
