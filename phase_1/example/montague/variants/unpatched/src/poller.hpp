#pragma once
#include "error.hpp"
#include <map>
#include <memory>
#include <sys/epoll.h>
const int max_events = 16;
class Serviceable;
class Poller {
public:
  Poller();
  ~Poller();
  int poll();
  void register_serviceable(std::shared_ptr<Serviceable> serv);
  void deregister_serviceable(std::shared_ptr<Serviceable> serv);
private:
  int epoll_fd;
  std::map<int, std::shared_ptr<Serviceable>> serviceables;
};
