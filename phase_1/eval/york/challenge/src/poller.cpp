#include "poller.hpp"

#include <array>
#include <sys/epoll.h>
#include <unistd.h>

#include "error.hpp"
#include "logger.hpp"
#include "serviceable.hpp"

Poller shared_poller = {};
constexpr size_t max_events = 16;

Poller::Poller() {
  epoll_fd = epoll_create(1);

  if (-1 == epoll_fd) {
    throw SystemError();
  }
}

Poller::~Poller() {
  close(epoll_fd);
}

void Poller::register_serviceable(ServiceablePtr serv) {
  shared_poller._register(serv);
}

void Poller::deregister_serviceable(ServiceablePtr serv) {
  shared_poller._deregister(serv);
}

void Poller::poll() {
  shared_poller._poll();
}

void Poller::_register(ServiceablePtr serv) {
  int input_fd = serv->input_fd();

  serviceables[input_fd] = serv;
  struct epoll_event evt = {
                            .events = EPOLLIN,
                            .data.fd = input_fd
  };

  if (0 != epoll_ctl(epoll_fd, EPOLL_CTL_ADD, input_fd, &evt)) {
    throw SystemError();
  }
}

void Poller::_deregister(ServiceablePtr serv) {
  int input_fd = serv->input_fd();
  auto got = serviceables.find(input_fd);
  if (serviceables.end() == got) {
    throw RuntimeError("tried to deregister unregistered serviceable");
  }

  serviceables.erase(got);

  if (0 != epoll_ctl(epoll_fd, EPOLL_CTL_DEL, input_fd, nullptr)) {
    throw SystemError();
  }
}

void Poller::_poll() {
  std::array<struct epoll_event, max_events> evs;
  int got_events = epoll_wait(epoll_fd, evs.data(), evs.size(), -1);

  LLL.info() << got_events << " events";

  for (int i = 0; i < got_events; i++) {
    struct epoll_event evt = evs[i];
    ServiceablePtr sxl = serviceables[evt.data.fd];

    if (0 != (evt.events & (EPOLLRDHUP | EPOLLHUP))) {
      LLL.info() << "fd " << evt.data.fd << " hung up";
      _deregister(sxl);
      continue;
    }

    try {
      LLL.info() << "fd " << evt.data.fd << " events " << evt.events;
      sxl->service(evt.data.fd);
      if (sxl->is_finished) {
        _deregister(sxl);
      }
    } catch (const RuntimeError& re) {
      LLL.error() << "fd " << evt.data.fd << " threw runtime error: "
                  << re.what();
      _deregister(sxl);
    }
  }
}
