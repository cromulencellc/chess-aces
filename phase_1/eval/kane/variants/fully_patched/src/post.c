#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "post.h"
#define MAX_HEADER_COUNT 32
#define MAX_POST_SIZE 2048
int get_cont_len(Header *headers) {
  for (int i = 0; i < MAX_HEADER_COUNT && headers[i].name != NULL; ++i) {
    if (strcmp(headers[i].name, "Content-Length") == 0) {
      return atoi(headers[i].value);
    }
  }
  return 0;
}
void populate_req_body(Request *req, FILE *in) {
  req->body = NULL;
  if (strcmp(req->method, "POST") == 0) {
    char b;
    int post_size = get_cont_len(req->headers);
    req->body = calloc(1, post_size + 1);
    for (int i = 0; i < post_size && i < MAX_POST_SIZE; ++i) {
      b = fgetc(in);
      req->body[i] = b;
      req->body[i + 1] = '\0';
    }
    printf("%s\n", req->body);
  }
}
