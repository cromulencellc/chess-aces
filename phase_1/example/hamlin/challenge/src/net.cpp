#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <ext/stdio_filebuf.h>
#include <iostream>

#include "assert.hpp"
#include "log.hpp"
#include "hton.hpp"
#include "net.hpp"

Net::Net(uint16_t p) : port(p) {
  lll("preparing to listen on port %d", port);
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  int sock_got = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                            &opt, sizeof(opt));
  assert_zero(sock_got);

  struct sockaddr_in address {
                              .sin_family = AF_INET,
                              .sin_addr.s_addr = INADDR_ANY,
                              .sin_port = hton(port)
  };

  int bind_got = bind(sock, (const struct sockaddr*)&address, sizeof(address));
  assert_zero(bind_got);

  int listen_got = listen(sock, 1);
  assert_zero(listen_got);

  struct sockaddr client_address;
  socklen_t client_address_len = sizeof(client_address);

  client_fd = accept(sock, &client_address, &client_address_len);
}

std::istream* Net::get_in() {
  auto in_buf = new __gnu_cxx::stdio_filebuf<char>(
                                                   dup(client_fd),
                                                   std::ios::binary | std::ios::in);
  return new std::istream(in_buf);
}

std::ostream* Net::get_out() {
  auto out_buf = new __gnu_cxx::stdio_filebuf<char>(
                                                    dup(client_fd),
                                                    std::ios::binary | std::ios::out);
  return new std::ostream(out_buf);
}
