#pragma once
#include "poller.hpp"
class Serviceable {
public:
  virtual void service(Poller &poller) = 0;
  virtual int fd() = 0;
};
