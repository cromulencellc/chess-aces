#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <malloc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "field.h"
#include "filter.h"
#include "main.h"
#include "stream.h"
#include "testbed.h"
#define SUCCESS 0
#define FAIL -1
int accept_fd;
filter *filter_root;
void sigintHandler(int info) {
  close(accept_fd);
  exit(0);
}
char data2[32];
char token[32];
char *dupbuff(const char *buff, unsigned int start, unsigned int end) {
  char *t = NULL;
  int l;
  if (!buff || end <= start) {
    goto end;
  }
  l = end - start;
  t = calloc(1, l + 1);
  if (!t) {
    goto end;
  }
  memcpy(t, buff + start, l);
end:
  return t;
}
int read_length(int fd, char *buffer, int length) {
  int index = 0;
  int bytes_read = 0;
  int bytes_left = length;
  if (!buffer) {
    goto end;
  }
  while (bytes_left) {
    bytes_read = read(fd, buffer + index, bytes_left);
    if (bytes_read <= 0) {
      goto end;
    }
    index += bytes_read;
    bytes_left -= bytes_read;
  }
end:
  return index;
}
void destroy_request(HTTPRequest *req) {
  field *t;
  if (!req) {
    goto end;
  }
  t = req->field_roots;
  while (t) {
    req->field_roots = t->next;
    if (t->field_name) {
      free(t->field_name);
    }
    if (t->field_data) {
      free(t->field_data);
    }
    free(t);
    t = req->field_roots;
  }
  if (req->request_uri) {
    free(req->request_uri);
  }
  if (req->dest_server) {
    free(req->dest_server);
  }
  if (req->content) {
    free(req->content);
  }
  if (req->response_string) {
    free(req->response_string);
  }
  free(req);
end:
  return;
}
HTTPRequest *tokenize_response(char *request, int length) {
  char *temp = NULL;
  HTTPRequest *req = NULL;
  stream *s = NULL;
  field *nf = NULL;
  char c;
  if (!request) {
    goto fail;
  }
  s = initstream(request, length);
  if (!s) {
    goto fail;
  }
  req = calloc(1, sizeof(HTTPRequest));
  if (!req) {
    goto fail;
  }
  temp = readuntil(s, '/');
  if (!temp) {
    goto fail;
  }
  if (strcasecmp(temp, "HTTP")) {
    free(temp);
    goto fail;
  }
  free(temp);
  incstream(s);
  temp = readuntil(s, '.');
  if (!temp) {
    goto fail;
  }
  req->major_version = atoi(temp);
  free(temp);
  incstream(s);
  temp = readuntil(s, ' ');
  if (!temp) {
    goto fail;
  }
  req->minor_version = atoi(temp);
  free(temp);
  incstream(s);
  temp = readuntil(s, ' ');
  if (!temp) {
    goto fail;
  }
  req->response_code = atoi(temp);
  free(temp);
  incstream(s);
  req->response_string = readuntil(s, '\r');
  if (!req->response_code) {
    goto fail;
  }
  incstream(s);
  if (readbyte(s, &c)) {
    goto fail;
  }
  if (c != '\n') {
    goto fail;
  }
  temp = readuntil(s, '\r');
  while (temp) {
    incstream(s);
    if (strlen(temp) == 0) {
      free(temp);
      if (readbyte(s, &c)) {
        goto fail;
      }
      if (c != '\n') {
        goto fail;
      }
      break;
    }
    if (readbyte(s, &c)) {
      free(temp);
      goto fail;
    }
    if (c != '\n') {
      free(temp);
      goto fail;
    }
    nf = parse_field(temp);
    free(temp);
    if (!nf) {
      goto fail;
    }
    if (strcasecmp(nf->field_name, "Content-Length") == 0) {
      req->content_length = atoi(nf->field_data);
      free(nf->field_data);
      free(nf->field_name);
      free(nf);
    } else {
      add_field(&req->field_roots, nf);
    }
    temp = readuntil(s, '\r');
  }
  free_stream(s);
  return req;
fail:
  free_stream(s);
  destroy_request(req);
  return NULL;
}
HTTPRequest *tokenize_request(char *request, int length) {
  char *end = NULL;
  char *current_line = NULL;
  char *method = NULL;
  char *temp = NULL;
  HTTPRequest *req = NULL;
  stream *s = NULL;
  field *nf = NULL;
  char c;
  if (!request) {
    goto fail;
  }
  s = initstream(request, length);
  if (!s) {
    goto fail;
  }
  req = calloc(1, sizeof(HTTPRequest));
  if (!req) {
    goto fail;
  }
  method = readuntil(s, ' ');
  if (!method) {
    goto fail;
  }
  incstream(s);
  if (strncasecmp(method, "get", end - current_line) == 0) {
    req->m = get;
  } else if (strncasecmp(method, "head", end - current_line) == 0) {
    req->m = head;
  } else if (strncasecmp(method, "post", end - current_line) == 0) {
    req->m = post;
  } else {
    free(method);
    goto fail;
  }
  free(method);
  req->request_uri = readuntil(s, ' ');
  if (!req->request_uri) {
    goto fail;
  }
  incstream(s);
  temp = readuntil(s, '/');
  if (!temp) {
    goto fail;
  }
  incstream(s);
  if (strcasecmp(temp, "HTTP") != 0) {
    free(temp);
    goto fail;
  }
  free(temp);
  temp = readuntil(s, '.');
  if (!temp) {
    goto fail;
  }
  incstream(s);
  req->major_version = atoi(temp);
  free(temp);
  if (req->major_version != 1) {
    goto fail;
  }
  temp = readuntil(s, '\r');
  if (!temp) {
    goto fail;
  }
  incstream(s);
  req->minor_version = atoi(temp);
  free(temp);
  if (req->minor_version != 1) {
    goto fail;
  }
  if (readbyte(s, &c)) {
    goto fail;
  }
  if (c != '\n') {
    goto fail;
  }
  temp = readuntil(s, '\r');
  while (temp) {
    incstream(s);
    if (strlen(temp) == 0) {
      free(temp);
      if (readbyte(s, &c)) {
        goto fail;
      }
      if (c != '\n') {
        goto fail;
      }
      break;
    }
    if (readbyte(s, &c)) {
      free(temp);
      goto fail;
    }
    if (c != '\n') {
      free(temp);
      goto fail;
    }
    nf = parse_field(temp);
    free(temp);
    if (!nf) {
      goto fail;
    }
    add_field(&req->field_roots, nf);
    temp = readuntil(s, '\r');
  }
  free_stream(s);
  return req;
fail:
  free_stream(s);
  destroy_request(req);
  return NULL;
}
int read_request(int fd, char **req_out) {
  stream *s = NULL;
  int flag = 0;
  int length = 0;
  char c;
  if (!req_out) {
    goto end;
  }
  s = init_stream_nd(512);
  if (!s) {
    goto end;
  }
  while (flag != 4) {
    if (read(fd, &c, 1) <= 0) {
      free(s->data);
      free(s);
      s = NULL;
      goto end;
    }
    if (c == '\r') {
      switch (flag) {
      case 0:
        flag = 1;
        break;
      case 1:
        break;
      case 2:
        flag = 3;
        break;
      default:
        flag = 1;
        break;
      }
    } else if (c == '\n') {
      switch (flag) {
      case 0:
        break;
      case 1:
        flag = 2;
        break;
      case 2:
        flag = 0;
        break;
      case 3:
        flag = 4;
        break;
      default:
        flag = 0;
        break;
      }
    } else {
      flag = 0;
    }
    add_bytes(s, &c, 1);
  }
  *req_out = s->data;
  length = s->index;
end:
  if (s) {
    free(s);
  }
  return length;
}
int get_dest_server_from_uri(HTTPRequest *req) {
  int result = FAIL;
  char *server = NULL;
  char *t = NULL;
  char *end = NULL;
  char *port = NULL;
  char *updated_uri = NULL;
  if (!req) {
    goto end;
  }
  if (!req->request_uri) {
    goto end;
  }
  t = req->request_uri;
  if (!strncmp(t, "http://", 7)) {
    t += 7;
  }
  end = strchr(t, ':');
  if (end) {
    server = dupbuff(t, 0, end - t);
    if (!server) {
      goto end;
    }
    t = end + 1;
    end = strchr(t, '/');
    if (end == t) {
      free(server);
      goto end;
    }
    if (end) {
      port = dupbuff(t, 0, end - t);
      updated_uri = strdup(end);
      free(req->request_uri);
      req->request_uri = updated_uri;
    } else {
      port = strdup(t);
      updated_uri = strdup("/");
      free(req->request_uri);
      req->request_uri = updated_uri;
    }
  } else {
    end = strchr(t, '/');
    if (end == t) {
      goto end;
    }
    if (end) {
      server = dupbuff(t, 0, end - t);
      updated_uri = strdup(end);
      free(req->request_uri);
      req->request_uri = updated_uri;
    } else {
      server = strdup(t);
      free(req->request_uri);
      req->request_uri = strdup("/");
    }
  }
  for (int i = 0; i < strlen(server); i++) {
    if (!isascii(server[i])) {
      free(server);
      free(port);
      goto end;
    }
  }
  if (server) {
  }
  if (port) {
    for (int i = 0; i < strlen(port); i++) {
      if (!isdigit(port[i])) {
        free(server);
        free(port);
        goto end;
      }
    }
    req->port = atoi(port);
    free(port);
  } else {
    req->port = 80;
  }
  req->dest_server = server;
  result = SUCCESS;
end:
  return result;
}
int send_string(int fd, char *s) {
  int result = FAIL;
  int length = 0;
  int bytes_written = 0;
  if (!s) {
    goto end;
  }
  length = strlen(s);
  while (bytes_written < length) {
    result = write(fd, s + bytes_written, length - bytes_written);
    if (result <= 0) {
      return FAIL;
    }
    bytes_written += result;
  }
end:
  return result;
}
char *get_method_string(enum method e) {
  switch (e) {
  case get:
    return "GET";
  case head:
    return "HEAD";
  case put:
    return "PUT";
  case delete:
    return "DELETE";
  case conn:
    return "CONNECT";
  case options:
    return "OPTIONS";
  case trace:
    return "TRACE";
  default:
    return "UNKNOWN";
  }
  return NULL;
}
char *construct_request(HTTPRequest *req) {
  stream *s;
  char *data = NULL;
  field *f = NULL;
  if (!req) {
    goto end;
  }
  s = init_stream_nd(512);
  if (!s) {
    goto end;
  }
  add_string(s, get_method_string(req->m));
  add_string(s, " ");
  add_string(s, req->request_uri);
  add_string(s, " HTTP/1.1\r\n");
  for (f = req->field_roots; f; f = f->next) {
    add_string(s, f->field_name);
    add_string(s, ": ");
    add_string(s, f->field_data);
    add_string(s, "\r\n");
  }
  add_string(s, "\r\n");
  data = s->data;
  free(s);
end:
  return data;
}
char *construct_response(HTTPRequest *req, int code, char *code_string,
                         char **output, int *out_len) {
  stream *s;
  field *f = NULL;
  if (!req || !code_string || !output || !out_len) {
    goto end;
  }
  s = init_stream_nd(512);
  if (!s) {
    goto end;
  }
  add_string(s, "HTTP/1.1 ");
  add_int(s, code);
  add_string(s, " ");
  add_string(s, code_string);
  add_string(s, "\r\n");
  for (f = req->field_roots; f; f = f->next) {
    add_string(s, f->field_name);
    add_string(s, ": ");
    add_string(s, f->field_data);
    add_string(s, "\r\n");
  }
  if (req->content_length > 0) {
    add_string(s, "Content-Length: ");
    add_int(s, req->content_length);
    add_string(s, "\r\n");
  }
  add_string(s, "\r\n");
  if (req->content_length > 0) {
    add_bytes(s, req->content, req->content_length);
  }
  *output = s->data;
  *out_len = s->index;
  free(s);
end:
  return *output;
}
int connect_to_server(char *server, int port) {
  int result = FAIL;
  struct addrinfo ai;
  struct addrinfo *rez = NULL;
  int fd;
  char cport[6];
  if (!server) {
    goto end;
  }
  memset(&ai, 0, sizeof(struct addrinfo));
  memset(cport, 0, 6);
  ai.ai_family = AF_INET;
  ai.ai_socktype = SOCK_STREAM;
  snprintf(cport, 6, "%d", port);
  result = getaddrinfo(server, cport, &ai, &rez);
  if (result) {
    printf("[ERROR] getaddrinfo(): '%s' failed: %s\n", server,
           gai_strerror(result));
    return result;
  }
  fd = socket(rez->ai_family, rez->ai_socktype, 0);
  if (fd < 0) {
    printf("[ERROR] socket(): %s\n", strerror(errno));
    result = FAIL;
    goto end;
  }
  if (connect(fd, rez->ai_addr, rez->ai_addrlen) < 0) {
    printf("[ERROR] connect(): %s\n", strerror(errno));
    result = FAIL;
    goto end;
  }
  freeaddrinfo(rez);
  result = fd;
end:
  return result;
}
int handle_client(int fd) {
  char *data = NULL;
  char *http_request_string = NULL;
  int http_req_length = 0;
  char *http_forward_request_string = NULL;
  char *http_proxy_reply = NULL;
  char *proxy_content = NULL;
  HTTPRequest *req = NULL;
  int result = FAIL;
  int server_fd = 0;
  int l = 0;
  HTTPRequest *proxy_response = NULL;
  http_req_length = read_request(fd, &http_request_string);
  if (http_request_string != NULL) {
    req = tokenize_request(http_request_string, http_req_length);
    free(http_request_string);
  } else {
    goto end;
  }
  if (!req) {
    send_string(fd, "HTTP/1.1 400 Bad Request\r\nServer: "
                    "Chess\r\nContent-Length: 5\r\n\r\naaaaa");
    goto end;
  }
  if (get_dest_server_from_uri(req) && !is_filter(req->request_uri)) {
    send_string(fd, "HTTP/1.1 400 Bad Request\r\nServer: "
                    "Chess\r\nContent-Length: 5\r\n\r\naaaaa");
    goto end;
  }
  if (is_filter(req->request_uri)) {
    if (add_filter(&filter_root,
                   parse_filter(get_field_data(&req->field_roots, "Filter")))) {
      send_string(fd, "HTTP/1.1 469 Filter Failed\r\nServer: "
                      "Chess\r\nContent-Length: 5\r\n\r\naaaaa");
    }
    goto end;
  }
  apply_filters(req, filter_root, 1);
  if (req->blocked) {
    send_string(fd, "HTTP/1.1 600 Proxy request blocked.\r\nServer: "
                    "Chess\r\nContent-Length: 5\r\n\r\naaaaa");
    goto end;
  }
  server_fd = connect_to_server(req->dest_server, req->port);
  if (server_fd < 0) {
    send_string(fd, "HTTP/1.1 500 Connection Failed\r\nServer: "
                    "Chess\r\nContent-Length: 5\r\n\r\naaaaa");
    goto end;
  }
  http_forward_request_string = construct_request(req);
  if (!http_forward_request_string) {
    send_string(fd, "HTTP/1.1 400 Bad Request\r\nServer: "
                    "Chess\r\nContent-Length: 5\r\n\r\naaaaa");
    goto end;
  }
  send_string(server_fd, http_forward_request_string);
  free(http_forward_request_string);
  l = read_request(server_fd, &http_proxy_reply);
  if (!http_proxy_reply) {
    send_string(fd, "HTTP/1.1 500 Connection Failed\r\nServer: "
                    "Chess\r\nContent-Length: 5\r\n\r\naaaaa");
    goto end;
  }
  proxy_response = tokenize_response(http_proxy_reply, l);
  free(http_proxy_reply);
  if (!proxy_response) {
    send_string(fd, "HTTP/1.1 500 Response Failed\r\nServer: "
                    "Chess\r\nContent-Length: 5\r\n\r\naaaaa");
    close(server_fd);
    goto end;
  }
  if (proxy_response->content_length > 0 && req->m != head) {
    proxy_content = calloc(1, proxy_response->content_length + 1);
    if (!proxy_content) {
      goto end;
    }
    proxy_response->content_length =
        read_length(server_fd, proxy_content, proxy_response->content_length);
    if (proxy_response->content_length <= 0) {
      free(proxy_content);
      goto end;
    }
    proxy_response->content = proxy_content;
  }
  close(server_fd);
  apply_filters(proxy_response, filter_root, 0);
  if (proxy_response->blocked) {
    send_string(fd, "HTTP/1.1 600 Proxy request filtered.\r\nServer: "
                    "Chess\r\nContent-Length: 5\r\n\r\naaaaa");
    goto end;
  }
  construct_response(proxy_response, 200, "OK", &data, &l);
  if (!data) {
    send_string(fd, "HTTP/1.1 500 Response Failed\r\nServer: "
                    "Chess\r\nContent-Length: 5\r\n\r\naaaaa");
    close(fd);
    goto end;
  }
  int written = 0;
  int total_written = 0;
  int to_write = l;
  while (total_written < l) {
    written = write(fd, data + total_written, to_write);
    if (written < 0) {
      printf("[ERROR] Failed to write data: %s\n", strerror(errno));
      free(data);
      data = NULL;
      close(fd);
      goto end;
    }
    total_written += written;
    to_write -= written;
  }
  free(data);
  data = NULL;
  close(fd);
  result = SUCCESS;
end:
  destroy_request(req);
  destroy_request(proxy_response);
  return result;
}
int accept_loop() {
  int conn_fd = 0;
  struct sockaddr_in ca;
  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;
  socklen_t ca_len = sizeof(struct sockaddr_in);
  memset(&ca, 0, sizeof(struct sockaddr_in));
  while (1) {
    conn_fd = accept(accept_fd, (struct sockaddr *)&ca, &ca_len);
    if (conn_fd < 0) {
      fprintf(stderr, "accept() failed: %s\n", strerror(errno));
      close(accept_fd);
      return -1;
    }
    handle_client(conn_fd);
    close(conn_fd);
  }
  return 0;
}
int setup_socket(int port) {
  int fd;
  struct sockaddr_in sa;
  int enable = 1;
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    fprintf(stderr, "socket() failed: %s\n", strerror(errno));
    exit(0);
  }
  setsockopt(fd, SOL_TCP, TCP_NODELAY, &enable, sizeof(enable));
  memset(&sa, 0, sizeof(struct sockaddr_in));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = INADDR_ANY;
  sa.sin_port = htons(port);
  if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    fprintf(stderr, "bind() failed: %s\n", strerror(errno));
    close(fd);
    exit(0);
  }
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
    fprintf(stderr, "setsockopt() failed: %s\n", strerror(errno));
    close(fd);
    exit(0);
  }
  if (listen(fd, 1024) < 0) {
    fprintf(stderr, "listen() failed: %s\n", strerror(errno));
    close(fd);
    exit(0);
  }
  fprintf(stderr, "[INFO] Listener socket on port: %d\n", port);
  accept_fd = fd;
  return fd;
}
void read_token() {
  int rfd;
  memset(token, 0, 32);
  rfd = open("/token", O_RDONLY);
  if (rfd < 0) {
    printf("[ERROR] Failed to open token\n");
    exit(1);
  }
  if (read(rfd, token, 32) < 32) {
    printf("[ERROR] Failed to read the correct number of bytes\n");
    close(rfd);
    exit(1);
  }
  close(rfd);
  return;
}
int main(int argc, char **argv) {
  int port;
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif
  memset(data2, 0, 32);
  read_token();
  char *t = getenv("PORT");
  if (t != NULL) {
    port = atoi(t);
  } else {
    port = 3004;
  }
  signal(SIGINT, sigintHandler);
  setup_socket(port);
  if (accept_fd < 0) {
    exit(1);
  }
  accept_loop();
  return 0;
}
