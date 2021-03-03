#pragma once
#include <iostream>
#include <optional>
#include <system_error>
#include "connection.hpp"
#include "error.hpp"
#include "serviceable.hpp"
class ListenerError : public std::system_error, public MontagueError {
public:
  ListenerError();
};
class Listener : public Serviceable {
public:
  Listener(uint16_t p);
  virtual ~Listener();
  virtual void service(Poller &);
  virtual int fd();
  uint16_t port;
  int sock;
};
