#pragma once
#include <iostream>
#include <memory>
#include <optional>
#include <sys/socket.h>
#include <vector>
#include "address.hpp"
#include "serviceable.hpp"
#include "uuid.hpp"
class Connection : public Serviceable {
public:
  Connection(int client_fd, struct sockaddr client_addr);
  virtual ~Connection();
  virtual void service(Poller &poller);
  virtual int fd();

private:
  int _fd;
  Address client_address;
  Uuid id = {};
};
