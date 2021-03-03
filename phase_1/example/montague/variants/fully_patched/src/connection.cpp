#include "connection.hpp"
#include "logger.hpp"
#include "transaction.hpp"
#include <ext/stdio_filebuf.h>
#include <unistd.h>
Connection::Connection(int client_fd, struct sockaddr client_addr)
    : _fd(client_fd), client_address(client_addr) {
  LLL.info() << "connection " << id << "open";
}
void Connection::service(Poller &p) {
  Transaction tx = {_fd, client_address};
  LLL.info() << "connection " << id << " serving tx " << tx.id;
  tx.service();
}
int Connection::fd() { return _fd; }
Connection::~Connection() {
  LLL.info() << "connection " << id << " closing";
  close(_fd);
}
