#ifndef HEADERS_H
#define HEADERS_H
#include <stdbool.h>
typedef struct Header {
  char *name;
  char *value;
  int count;
} Header;
Header *headers_parse(FILE *in);
void print_header(Header header);
void destroy_headers(Header *headers);
#endif
