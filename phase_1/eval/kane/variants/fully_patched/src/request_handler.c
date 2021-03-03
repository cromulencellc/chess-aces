#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "authenticate.h"
#include "get.h"
#include "request.h"
#include "response.h"
#include "role_table.h"
#define ALLOWED_PATH "/wiki/cache/"
#define QUERY_PATH "/wiki"
#define MAX_REQUEST_SIZE 200
void request_handler(FILE *in, FILE *out, int fd, RoleTable *rt) {
  struct timeval t1, t2;
  gettimeofday(&t1, NULL);
  Request req = request_parse(in);
  req.time_elapsed = 0;
  if (req.valid == false) {
    destroy_request(&req);
    return;
  }
  print_request(req);
  Header role_header = find_role_tag(req.headers);
  if (role_header.name != NULL && role_header.value != NULL) {
    if (strcmp(role_header.value, " ADMIN") == 0) {
      if (send_authorization_request(out, req, rt)) {
        rt = generate_rtable(rt, role_header);
      } else {
        return;
      }
    } else {
      rt = generate_rtable(rt, role_header);
    }
  }
  char *path_copy = strdup(req.path);
  path_copy[12] = '\0';
  if (strcmp(ALLOWED_PATH, path_copy) != 0 &&
      strcmp(QUERY_PATH, path_copy) != 0) {
    fprintf(stderr, "Invalid request returning...\n");
    free(path_copy);
    path_copy = NULL;
    return;
  }
  free(path_copy);
  path_copy = NULL;
  Response rep = assemble_response(req, out, rt);
  gettimeofday(&t2, NULL);
  req.time_elapsed = (double)(t2.tv_usec - t1.tv_usec) / 1000000 +
                     (double)(t2.tv_sec - t1.tv_sec);
  if (rep.exit == false) {
    print_response(rep);
  }
  send_response(&rep, out);
  destroy_request(&req);
  destroy_response(&rep);
}
