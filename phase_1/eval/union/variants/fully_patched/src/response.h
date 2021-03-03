#ifndef RESPONSE_H
#define RESPONSE_H
#include <stdbool.h>
#include "header.h"
#include "request.h"
typedef struct Stat_Line {
  char *http_ver;
  char *stat_code;
} Stat_Line;
typedef struct Response {
  Stat_Line stat_line;
  Header *headers;
  char *entity_body;
  bool exit;
} Response;
void print_response(Response rep);
void print_status_line(Stat_Line stl);
Stat_Line build_status_line(Request req);
Response assemble_response(Request *req, FILE *out);
Header build_date();
Header build_age();
Header build_connection();
Header build_cookie(Request r);
void build_entity_body(Response *rep, Request *req);
void send_response(Response *rep, FILE *out);
void destroy_response(Response *rep);
#endif
