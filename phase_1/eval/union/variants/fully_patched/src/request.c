#define _GNU_SOURCE
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "get.h"
#include "header.h"
#include "post.h"
#include "request.h"
#define MAX_METHOD_SIZE 8
#define MAX_PATH_SIZE 512
#define MAX_QUERY_SIZE 512
#define MAX_VERSION_SIZE 16
#define NUM_REQUEST_MEMBERS 4
Request request_parse(FILE *in) {
  Request req;
  req.method = NULL;
  req.path = NULL;
  req.query = NULL;
  req.version = NULL;
  req.valid = true;
  req.error_number = 0;
  req.query_flag = false;
  req.headers = NULL;
  req.method = calloc(1, MAX_METHOD_SIZE + 1);
  for (size_t m_curr = 0; m_curr < MAX_METHOD_SIZE; ++m_curr) {
    char c = fgetc(in);
    if (c == ' ') {
      break;
    }
    if (!isalpha(c)) {
      req.valid = false;
      return req;
    }
    if (!isupper(c)) {
      req.valid = false;
      return req;
    }
    req.method[m_curr] = c;
  }
  req.path = calloc(1, MAX_PATH_SIZE + 1);
  for (size_t p_curr = 0; p_curr < MAX_PATH_SIZE; ++p_curr) {
    char c = fgetc(in);
    if (c == ' ') {
      req.query = NULL;
      break;
    }
    if (c == '?') {
      req.query = calloc(1, MAX_QUERY_SIZE + 1);
      break;
    }
    req.path[p_curr] = c;
  }
  if (req.query != NULL) {
    req.query_flag = true;
    for (size_t q_curr = 0; q_curr < MAX_QUERY_SIZE; ++q_curr) {
      char c = fgetc(in);
      if (c == ' ') {
        break;
      }
      req.query[q_curr] = c;
    }
  }
  if (req.query_flag == true) {
    if (!sanitize(req.query)) {
      printf("There was a problem with the query\n");
      req.valid = false;
      return req;
    }
  }
  req.version = calloc(1, MAX_VERSION_SIZE + 1);
  for (size_t v_curr = 0; v_curr < MAX_VERSION_SIZE; v_curr++) {
    char c = fgetc(in);
    if (c == '\r') {
      break;
    }
    req.version[v_curr] = c;
  }
  if (!expect_lf(in)) {
    req.valid = false;
    return req;
  }
  req.headers = headers_parse(in);
  if (req.headers != NULL) {
    populate_req_body(&req, in);
  }
  return req;
}
bool expect_lf(FILE *f) {
  if (fgetc(f) != '\n') {
    return false;
  }
  return true;
}
int sanitize(char *query) {
  if (strlen(query) < 3) {
    printf("Invalid Query\n");
    return 0;
  }
  int i = 0;
  for (i = 0; i < strlen(query); ++i) {
    if (!isalpha(query[i]) && !isdigit(query[i]) && query[i] != '_' &&
        query[i] != '&' && query[i] != '=') {
      fprintf(stderr, "[SANITIZE] The query is not valid as provided.\n");
      fprintf(stderr, "[SANITIZE] Problem character is: %c\n", query[i]);
      return 0;
    }
  }
  return 1;
}
void destroy_request(Request *request) {
  if (!request) {
    fprintf(stderr, "Request already empty");
    return;
  }
  if (request->method) {
    free(request->method);
    request->method = NULL;
  }
  if (request->path) {
    free(request->path);
    request->path = NULL;
  }
  if (request->query) {
    free(request->query);
    request->query = NULL;
  }
  if (request->version) {
    free(request->version);
    request->version = NULL;
  }
  destroy_headers(request->headers);
}
void print_request(Request request) {
  printf("METHOD: %s\n\n", request.method);
  printf("PATH: %s\n\n", request.path);
  printf("QUERY: %s\n\n", request.query);
  printf("VERSION: %s\n\n", request.version);
}
