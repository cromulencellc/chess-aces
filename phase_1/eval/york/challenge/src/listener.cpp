#include "listener.hpp"

#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include "address.hpp"
#include "error.hpp"
#include "hton.hpp"
#include "logger.hpp"

#include "http/connection.hpp"

Listener::Listener(uint16_t port_number) : port(port_number) {
  LLL.info() << "preparing to listen on tcp port " << port;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sock) throw SystemError();

  int opt = 1;
  int sock_got = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                            &opt, sizeof(opt));
  if (0 != sock_got) throw SystemError();
  sock_got = setsockopt(sock, SOL_TCP, TCP_NODELAY,
                        &opt, sizeof(opt));
  if (0 != sock_got) throw SystemError();

  struct sockaddr_in address {
                              .sin_family = AF_INET,
                              .sin_addr.s_addr = INADDR_ANY,
                              .sin_port = hton(port)
  };

  int bind_got = bind(sock, (const struct sockaddr*)&address, sizeof(address));
  if (0 != bind_got) throw SystemError();

  int listen_got = listen(sock, 1);
  if (0 != listen_got) throw SystemError();

  LLL.info() << "listening on socket fd " << sock;
}

Listener::~Listener() {
  int close_got = close(sock);
  if (0 != close_got) {
    LLL.fatal() << "totally failed to close listener, lmao";
  }
}

void Listener::service(int fd) {
  if (fd != sock) {
    throw RuntimeError("tried to listen on fd " +
                       std::to_string(fd) +
                       " but sock is " +
                       std::to_string(sock));
  }

  Address client_address;
  socklen_t client_address_len = sizeof(client_address);

  int client_fd = ::accept(sock, &client_address, &client_address_len);
  int flags = fcntl(client_fd, F_GETFL, 0);
  if (-1 == flags) throw SystemError();
  int fcntl_got = fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
  if (-1 == fcntl_got) throw SystemError();

  LLL.info() << "accepted connection from "
             << client_address
             << " on fd "
             << client_fd;

  std::shared_ptr<http::Connection> conn =
    std::make_shared<http::Connection>(client_fd, client_address);
  Poller::register_serviceable(conn);
}

int Listener::input_fd() {
  return sock;
}
