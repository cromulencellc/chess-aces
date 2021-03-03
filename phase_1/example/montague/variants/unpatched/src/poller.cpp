#include "poller.hpp"
#include "serviceable.hpp"
#include "error.hpp"
#include "logger.hpp"
#include <sys/epoll.h>
#include <unistd.h>
Poller::Poller() {
  epoll_fd = epoll_create(1);
  if (-1 == epoll_fd) {
    throw MontagueSystemError();
  }
}
Poller::~Poller() { close(epoll_fd); }
int Poller::poll() {
  std::array<struct epoll_event, max_events> evs;
  LLL.info() << "polling " << serviceables.size() << " fds";
  int got_events = epoll_wait(epoll_fd, evs.data(), evs.size(), -1);
  for (size_t i = 0; i < got_events; i++) {
    struct epoll_event evt = evs[i];
    std::shared_ptr<Serviceable> sxl = serviceables[evt.data.fd];
    if (0 != (evt.events & (EPOLLRDHUP | EPOLLHUP))) {
      LLL.info() << "fd " << evt.data.fd << " hung up";
      deregister_serviceable(sxl);
      continue;
    }
    try {
      sxl->service(*this);
    } catch (const MontagueRuntimeError &mre) {
      LLL.error() << "fd " << evt.data.fd
                  << " threw runtime error: " << mre.what();
      deregister_serviceable(sxl);
    }
  }
  return got_events;
}
void Poller::register_serviceable(std::shared_ptr<Serviceable> serv) {
  int serv_fd = serv->fd();
  serviceables[serv_fd] = serv;
  struct epoll_event evt = {.events = EPOLLIN, .data.fd = serv_fd};
  if (0 != epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_fd, &evt)) {
    throw MontagueSystemError();
  }
}
void Poller::deregister_serviceable(std::shared_ptr<Serviceable> serv) {
  int serv_fd = serv->fd();
  auto got = serviceables.find(serv_fd);
  if (serviceables.end() == got) {
    throw MontagueRuntimeError("tried to deregister unregistered serviceable");
  }
  serviceables.erase(got);
  if (0 != epoll_ctl(epoll_fd, EPOLL_CTL_DEL, serv_fd, nullptr)) {
    throw MontagueSystemError();
  }
}
