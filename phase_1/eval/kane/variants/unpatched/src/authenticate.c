#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "authenticate.h"
#include "base64.h"
#include "line_state_table.h"
#include "request.h"
#include "role_table.h"
#include "user_list.h"
bool send_authorization_request(FILE *out, Request req, RoleTable *rt) {
  printf("\n\n\nENTERING AUTH\n\n\n");
  UserList *ulist = retrieve_user_list();
  int i = 0;
  for (i = 0; req.headers[i].name != NULL && i < 32; ++i) {
    if (strcasecmp(req.headers[i].name, "Cookie") == 0) {
      char *holder = strtok(req.headers[i].value, " ");
      if (rt->entries[2].gen == true) {
        if (strncmp(holder, rt->entries[2].cookie, 32) == 0) {
          return true;
        }
      }
    }
    if (strcasecmp(req.headers[i].name, "Authorization") == 0) {
      char *auth_head_cpy = strdup(req.headers[i].value);
      printf("\nauth_head_cpy: %s\n\n", auth_head_cpy);
      char *to_decode = strtok(auth_head_cpy, " ");
      printf("\nto_decode: %s\n\n", auth_head_cpy);
      to_decode = strtok(NULL, " ");
      printf("\nto_decode after strok: %s\n\n", to_decode);
      char *decoded = decode_base64(to_decode);
      printf("\ndecoded: %s\n\n", decoded);
      char *username;
      char *password;
      if ((username = strtok(decoded, ":")) == NULL) {
        fprintf(stderr, "AUTH: USERNAME ERROR\n");
        return false;
      }
      printf("Username: %s\n", username);
      if ((password = strtok(NULL, "\r")) == NULL) {
        fprintf(stderr, "AUTH: PASSWORD ERROR\n");
        return false;
      }
      printf("Password: %s\n", password);
      int j = 0;
      for (j = 0; j < ulist->count; ++j) {
        if (strcmp(username, ulist->users[j]->username) == 0 &&
            strcmp(password, ulist->users[j]->password) == 0 &&
            strcmp(ulist->users[j]->type, "ADMIN") == 0) {
          free(auth_head_cpy);
          auth_head_cpy = NULL;
          return true;
        }
      }
    }
  }
  char *denied_line = "HTTP/1.1 401 Unauthorized";
  char *w_auth_header = "WWW-Authenticate: Basic realm=Mordor";
  fprintf(out, "%s\r\n%s\r\n\r\n", denied_line, w_auth_header);
  fprintf(stderr, "Failed to auth\n");
  return false;
}
