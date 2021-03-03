#include <cerrno>
#include <cstdlib>
#include <ext/stdio_filebuf.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "logger.hpp"
#include "address.hpp"
#include "hton.hpp"
#include "listener.hpp"
#define throw_nonzero(val)                                                     \
  {                                                                            \
    if (0 != (val))                                                            \
      throw ListenerError();                                                   \
  }
ListenerError::ListenerError() {
  std::system_error{errno, std::system_category()};
}
Listener::Listener(uint16_t p) : port(p) {
  LLL.info() << "preparing to listen on tcp " << p;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  int sock_got = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                            sizeof(opt));
  throw_nonzero(sock_got);
  struct sockaddr_in address {
    .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = hton(port)
  };
  int bind_got = bind(sock, (const struct sockaddr *)&address, sizeof(address));
  throw_nonzero(bind_got);
  int listen_got = listen(sock, 1);
  throw_nonzero(listen_got);
  LLL.info() << "listening on socket fd " << sock;
}
Listener::~Listener() { close(sock); }
void Listener::service(Poller &p) {
  Address client_address;
  socklen_t client_address_len = sizeof(client_address);
  int client_fd = ::accept(sock, &client_address, &client_address_len);
  LLL.info() << "accepted connection from " << client_address << " on fd "
             << client_fd;
  std::shared_ptr<Connection> conn =
      std::make_shared<Connection>(client_fd, client_address);
  p.register_serviceable(conn);
}
int Listener::fd() { return sock; }
