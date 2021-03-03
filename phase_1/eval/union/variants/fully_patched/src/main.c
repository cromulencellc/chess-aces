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
#include "request.h"
#include "request_handler.h"
#include "testbed.h"
#define MAX_REQUEST 4096
#define FDS_COUNT 1
#define PORT 3008
int main() {
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif
  int socket_id = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_id < 0) {
    perror("Failure in socket creation\n");
    exit(EXIT_FAILURE);
  }
  int opt = 1;
  int sockopt = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                           &opt, sizeof(opt));
  if (sockopt != 0) {
    perror("MAIN OPTIONS FAILED...\n");
    exit(EXIT_FAILURE);
  }
  struct sockaddr_in bindings;
  int port = atoi(getenv("PORT"));
  bindings.sin_family = AF_INET;
  bindings.sin_port = htons(port);
  bindings.sin_addr.s_addr = INADDR_ANY;
  int bind_var =
      bind(socket_id, (struct sockaddr *)&bindings, sizeof(bindings));
  if (bind_var != 0) {
    perror("Failure in the main binding...\n");
    exit(EXIT_FAILURE);
  }
  int listen_var = listen(socket_id, 0);
  if (listen_var != 0) {
    printf("Failure in the main listening...\n");
    return 1;
  }
  struct sockaddr address;
  socklen_t address_len = sizeof(address);
  while (1) {
    struct pollfd fds[FDS_COUNT];
    fds[0].fd = socket_id;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    printf("\nListening for incoming connections\n");
    int ret;
    ret = poll(fds, FDS_COUNT, -1);
    if (fds[0].revents & POLLIN) {
      int main_accept = accept(socket_id, &address, &address_len);
      FILE *in = fdopen(dup(main_accept), "r");
      FILE *out = fdopen(dup(main_accept), "w");
      request_handler(in, out);
      fclose(out);
      fclose(in);
      close(main_accept);
    }
  }
  return 0;
}
