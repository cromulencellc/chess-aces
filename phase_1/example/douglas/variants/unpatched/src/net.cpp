#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <ext/stdio_filebuf.h>
#include <iostream>

#include "log.h"
#include "net.h"

Net::Net(int p) {
  port = p;
  lll("preparing to listen on port %d\n", p);
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  int sock_got = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                            sizeof(opt));
  if (0 != sock_got) {
    lll("got %x from setsockopt, expected 0\n", sock_got);
    std::exit(-1);
  }

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  int bind_got = bind(sock, (const struct sockaddr*)&address, sizeof(address));
  if (0 != bind_got) {
    lll("got %x from bind, expected 0\n", bind_got);
    std::exit(-1);
  }

  int listen_got = listen(sock, 1);
  if (0 != listen_got) {
    lll("got %x from listen, expected 0\n", listen_got);
    std::exit(-1);
  }

  struct sockaddr client_address;
  socklen_t client_address_len = sizeof(client_address);

  client_fd = accept(sock, &client_address, &client_address_len);
}

std::istream* Net::get_in() {
  auto in_buf = new __gnu_cxx::stdio_filebuf<char>(
      dup(client_fd), std::ios::binary | std::ios::in);
  return new std::istream(in_buf);
}

std::ostream* Net::get_out() {
  auto out_buf = new __gnu_cxx::stdio_filebuf<char>(
      dup(client_fd), std::ios::binary | std::ios::out);
  return new std::ostream(out_buf);
}
