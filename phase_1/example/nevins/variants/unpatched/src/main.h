#ifndef __MAIN__
#define __MAIN__
enum method { get = 0, head, post, put, delete, conn, options, trace };
typedef struct HTTPRequest {
  enum method m;
  char *request_uri;
  int major_version;
  int minor_version;
  int response_code;
  char *response_string;
  char *content;
  int content_length;
  int blocked;
  char *dest_server;
  int port;
  field *field_roots;
} HTTPRequest;
#endif