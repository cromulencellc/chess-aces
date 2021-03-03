#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "admin_console.h"
#include "authenticate.h"
#include "header.h"
#include "html_writer.h"
#include "lru_queue.h"
#include "request.h"
#include "request_handler.h"
#include "role_table.h"
#include "testbed.h"
#define MAX_REQUEST 4096
#define ADMIN_PORT 1234
#define FDS_COUNT 3
#define NUMBER_OF_ROLES 3
int main() {
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif
  int socket_id = socket(AF_INET, SOCK_STREAM, 0);
  int admin_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_id < 0) {
    perror("Failure in socket creation\n");
    exit(EXIT_FAILURE);
  }
  if (admin_sock < 0) {
    perror("Failure in admin socket creation\n");
    exit(EXIT_FAILURE);
  }
  int opt = 1;
  int sockopt = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                           &opt, sizeof(opt));
  if (sockopt != 0) {
    perror("MAIN OPTIONS FAILED...\n");
    exit(EXIT_FAILURE);
  }
  int adminopt = setsockopt(admin_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                            &opt, sizeof(opt));
  if (adminopt != 0) {
    perror("ADMIN OPTIONS FAILED...\n");
    exit(EXIT_FAILURE);
  }
  struct sockaddr_in bindings, admin_binding;
  int port = atoi(getenv("HTTP_PORT"));
  bindings.sin_family = AF_INET;
  bindings.sin_port = htons(port);
  bindings.sin_addr.s_addr = INADDR_ANY;
  int admin_port = atoi(getenv("ADMIN_PORT"));
  admin_binding.sin_family = AF_INET;
  admin_binding.sin_port = htons(admin_port);
  admin_binding.sin_addr.s_addr = INADDR_ANY;
  int bind_var =
      bind(socket_id, (struct sockaddr *)&bindings, sizeof(bindings));
  int admin_bind = bind(admin_sock, (struct sockaddr *)&admin_binding,
                        sizeof(admin_binding));
  if (bind_var != 0) {
    perror("Failure in the main binding...\n");
    exit(EXIT_FAILURE);
  }
  if (admin_bind != 0) {
    perror("Failure in the admin binding...\n");
    exit(EXIT_FAILURE);
  }
  LRU_Queue *queue = create_queue();
  textHtmlMatchPair();
  intialize_queue(queue);
  int listen_var = listen(socket_id, 0);
  int admin_listen = listen(admin_sock, 0);
  if (listen_var != 0) {
    printf("Failure in the main listening...\n");
    return 1;
  }
  if (admin_listen != 0) {
    printf("Failure in the admin listening...\n");
    return 1;
  }
  struct sockaddr address;
  socklen_t address_len = sizeof(address);
  RoleTable *rtable = malloc(sizeof(RoleTable));
  rtable->entries[0].gen = false;
  rtable->entries[1].gen = false;
  rtable->entries[2].gen = false;
  int ret = 0;
  while (1) {
    struct pollfd fds[FDS_COUNT];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    fds[1].fd = socket_id;
    fds[1].events = POLLIN;
    fds[1].revents = 0;
    fds[2].fd = admin_sock;
    fds[2].events = POLLIN;
    fds[2].revents = 0;
    printf("\nListening for incoming connections\n");
    ret = poll(fds, FDS_COUNT, -1);
    if (fds[0].revents & POLLIN) {
      char kill[2];
      read(0, kill, 1);
      if (kill[0] == 'k') {
        return 0;
      } else {
        fds[0].revents = 0;
      }
    }
    if (fds[1].revents & POLLIN) {
      int main_accept = accept(socket_id, &address, &address_len);
      FILE *in = fdopen(dup(main_accept), "r");
      FILE *out = fdopen(dup(main_accept), "w");
      request_handler(in, out, main_accept, rtable);
      fclose(out);
      fclose(in);
      close(main_accept);
    }
    if (fds[2].revents & POLLIN) {
      int admin_accept = accept(admin_sock, &address, &address_len);
      FILE *in = fdopen(dup(admin_accept), "r");
      FILE *out = fdopen(dup(admin_accept), "w");
      Request req = request_parse(in);
      if (req.headers == NULL) {
        break;
      }
      if (send_authorization_request(out, req, rtable)) {
        start_admin_console(admin_accept);
      }
      close(admin_accept);
      fclose(in);
      fclose(out);
    }
  }
  return 0;
}
