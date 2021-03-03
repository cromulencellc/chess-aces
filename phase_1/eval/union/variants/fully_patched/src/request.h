#ifndef REQUEST_H
#define REQUEST_H
#include "header.h"
#include <stdbool.h>
typedef struct Request {
  char *method;
  char *path;
  char *query;
  char *version;
  bool query_flag;
  Header *headers;
  size_t header_len;
  char body[2048];
  bool valid;
  int error_number;
  double time_elapsed;
} Request;
Request request_parse(FILE *in);
int sanitize(char *query);
bool expect_lf(FILE *f);
void print_request(Request request);
void destroy_request(Request *request);
#endif
