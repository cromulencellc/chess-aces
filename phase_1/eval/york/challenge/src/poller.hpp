#pragma once

#include <map>
#include <memory>

class Serviceable;
using ServiceablePtr = std::shared_ptr<Serviceable>;

class Poller {
public:

  Poller();
  ~Poller();

  static void register_serviceable(ServiceablePtr serv);
  static void deregister_serviceable(ServiceablePtr serv);

  static void poll();
private:

  int epoll_fd;

  std::map<int, ServiceablePtr> serviceables;

  void _register(ServiceablePtr serv);
  void _deregister(ServiceablePtr serv);
  void _poll();
};
