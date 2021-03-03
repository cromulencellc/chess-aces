#include "listener.hpp"

#include "address.hpp"
#include "error.hpp"
#include "hton.hpp"
#include "logger.hpp"

#include <cstdlib>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#define throw_nonzero(val) {if (0 != (val)) throw ListenerError();}

class ListenerError : public SystemError {};

Listener::Listener(uint16_t listen_port) {
  LLL.info() << "preparing to listen on tcp " << listen_port;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  int sock_got = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                            &opt, sizeof(opt));

  throw_nonzero(sock_got);

  sock_got = setsockopt(sock, SOL_TCP, TCP_NODELAY, &opt, sizeof(opt));

  throw_nonzero(sock_got);

  address::Address4 address {
                             .sin_family = AF_INET,
                             .sin_addr.s_addr = INADDR_ANY,
                             .sin_port = hton(listen_port)
  };

  int bind_got = bind(sock, (const Address*)&address, sizeof(address));
  throw_nonzero(bind_got);

  int listen_got = listen(sock, 1);
  throw_nonzero(listen_got);
  LLL.info() << "listening on socket fd " << sock;
}

Io Listener::accept() {
  Address client_address;
  socklen_t client_address_len = sizeof(client_address);

  int client_fd = ::accept(sock, &client_address, &client_address_len);

  LLL.info() << "accepted connection from " << client_address;

  std::string client_address_s = to_string(client_address);
  client_address_s.push_back('\n');

  ::write(client_fd, client_address_s.data(), client_address_s.size());

  return {client_fd};

}
