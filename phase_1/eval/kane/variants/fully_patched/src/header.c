#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "request.h"
#define MAX_HEADER_COUNT 32
#define MAX_HEADER_NAME_LEN 64
#define MAX_HEADER_VALUE_LEN 1024
Header *headers_parse(FILE *in) {
  Header *headers = calloc(MAX_HEADER_COUNT, sizeof(Header));
  size_t h_i = 0;
  for (h_i = 0; h_i < MAX_HEADER_COUNT; ++h_i) {
    headers[h_i].name = calloc(1, MAX_HEADER_NAME_LEN + 1);
    size_t h_name_curr = 0;
    size_t h_value_curr = 0;
    char c = '\0';
    for (; h_name_curr < MAX_HEADER_NAME_LEN; ++h_name_curr) {
      c = fgetc(in);
      if (c == ':') {
        break;
      } else if (c == '\r') {
        break;
      }
      headers[h_i].name[h_name_curr] = c;
    }
    if (h_name_curr == 0 && c == ':') {
      return NULL;
    } else if (h_name_curr == 0 && c == '\r') {
      if (!expect_lf(in))
        return NULL;
      return headers;
    }
    headers[h_i].value = calloc(1, MAX_HEADER_VALUE_LEN + 1);
    for (; h_value_curr < MAX_HEADER_VALUE_LEN; ++h_value_curr) {
      c = fgetc(in);
      if (c == '\r') {
        break;
      }
      headers[h_i].value[h_value_curr] = c;
    }
    if (!expect_lf(in)) {
      return NULL;
    }
    char r = fgetc(in);
    char n = fgetc(in);
    if (r == '\r' && n == '\n') {
      break;
    } else {
      ungetc(n, in);
      ungetc(r, in);
    }
    fprintf(stderr, "headers: %s: %s\n", headers[h_i].name, headers[h_i].value);
  }
  fprintf(stderr, "headers: %s: %s\n", headers[h_i].name, headers[h_i].value);
  return headers;
}
void print_header(Header header) {
  printf("%s ", header.name);
  printf("%s\n", header.value);
}
void destroy_headers(Header *headers) {
  if (!headers) {
    fprintf(stderr, "Headers is empty\n");
    return;
  }
  for (int i = 0; headers[i].name != NULL && headers[i].name != NULL; ++i) {
    if (headers[i].name) {
      free(headers[i].name);
      headers[i].name = NULL;
    }
    if (headers[i].value) {
      free(headers[i].value);
      headers[i].value = NULL;
    }
  }
}
