#ifndef HTTP_REQUEST_ENTRY_H
#define HTTP_REQUEST_ENTRY_H
#include "response.h"
#include <stdbool.h>
typedef struct HTTP_Request_Entry {
  char *date;
  char *method;
  char *resource_accessed;
  char *http_version;
  char *stat_line_result;
} HTTP_Request_Entry;
void create_log();
HTTP_Request_Entry *intialize_HTTP_Request();
HTTP_Request_Entry *create_entry(Response *resp, Request *request);
void record_entry(HTTP_Request_Entry *entry);
void print_HTTP_Request_Entry(HTTP_Request_Entry *entry);
void destroy_HTTP_Request_Entry(HTTP_Request_Entry *entry);
#endif
