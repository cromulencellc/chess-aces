#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "base64.h"
#include "cookie_generation.h"
#include "get_query.h"
#include "header.h"
#include "post.h"
#include "request.h"
#include "response.h"
#include "time.h"
#define SERV_HEADER_COUNT 4
#define MAX_HEADER_COUNT 32
#define MAX_FILE_SIZE 4096
#define DAY_LEN 3
#define SIG_FIGS 6
#define ROOT_DIR "/data"
#define HTTP_VER " HTTP/1.1\r\n"
#define OK_CODE "200 OK"
#define BAD_REQUEST "400 BAD REQUEST"
#define FORB_CODE "403 FORBIDDEN"
#define NOT_FOUND "404 NOT FOUND"
#define METHOD_NOT_ALLOWED "405 METHOD NOT ALLOWED"
void print_response(Response rep) {
  print_status_line(rep.stat_line);
  for (int i = 0; i < SERV_HEADER_COUNT; ++i) {
    print_header(rep.headers[i]);
  }
  if (rep.entity_body != NULL) {
    printf("entity_body: %s\n", rep.entity_body);
  }
}
void print_status_line(Stat_Line stl) {
  printf("%s %s\n", stl.http_ver, stl.stat_code);
}
Stat_Line build_status_line(Request req) {
  Stat_Line stat_line;
  stat_line.http_ver = calloc(1, strlen(HTTP_VER) + 1);
  if (req.error_number == 0) {
    stat_line.stat_code = calloc(1, strlen(OK_CODE) + 1);
    strncpy(stat_line.http_ver, HTTP_VER, strlen(HTTP_VER));
    strncpy(stat_line.stat_code, OK_CODE, strlen(OK_CODE));
  } else if (req.error_number == 1) {
    stat_line.stat_code = calloc(1, strlen(BAD_REQUEST) + 1);
    strncpy(stat_line.http_ver, HTTP_VER, strlen(HTTP_VER));
    strncpy(stat_line.stat_code, BAD_REQUEST, strlen(BAD_REQUEST));
  } else if (req.error_number == 2) {
    stat_line.stat_code = calloc(1, strlen(FORB_CODE) + 1);
    strncpy(stat_line.http_ver, HTTP_VER, strlen(HTTP_VER));
    strncpy(stat_line.stat_code, FORB_CODE, strlen(FORB_CODE));
  } else if (req.error_number == 3) {
    stat_line.stat_code = calloc(1, strlen(METHOD_NOT_ALLOWED) + 1);
    strncpy(stat_line.http_ver, HTTP_VER, strlen(HTTP_VER));
    strncpy(stat_line.stat_code, METHOD_NOT_ALLOWED,
            strlen(METHOD_NOT_ALLOWED));
  } else if (req.error_number == 4) {
    stat_line.stat_code = calloc(1, strlen(NOT_FOUND) + 1);
    strncpy(stat_line.http_ver, HTTP_VER, strlen(HTTP_VER));
    strncpy(stat_line.stat_code, NOT_FOUND, strlen(NOT_FOUND));
  }
  printf("%s %s\n", stat_line.stat_code, stat_line.http_ver);
  return stat_line;
}
Response assemble_response(Request *req, FILE *out) {
  Response response;
  response.headers = NULL;
  response.entity_body = NULL;
  response.exit = false;
  if (strncmp(req->method, "GET", strlen("GET")) == 0) {
    build_entity_body(&response, req);
  }
  response.stat_line = build_status_line(*req);
  response.headers = calloc(MAX_HEADER_COUNT, sizeof(Header));
  response.headers[0] = build_date();
  response.headers[1] = build_age(*req);
  response.headers[2] = build_connection();
  response.headers[3] = build_cookie(*req);
  return response;
}
Header build_date() {
  time_t t = time(NULL);
  struct tm time = *gmtime(&t);
  char day[4];
  switch (time.tm_wday) {
  case 0:
    strcpy(day, "Sun");
    break;
  case 1:
    strcpy(day, "Mon");
    break;
  case 2:
    strcpy(day, "Tue");
    break;
  case 3:
    strcpy(day, "Wed");
    break;
  case 4:
    strcpy(day, "Thu");
    break;
  case 5:
    strcpy(day, "Fri");
    break;
  case 6:
    strcpy(day, "Sat");
    break;
  }
  char mon[4];
  switch (time.tm_mon) {
  case 0:
    strcpy(mon, "Jan");
    break;
  case 1:
    strcpy(mon, "Feb");
    break;
  case 2:
    strcpy(mon, "Mar");
    break;
  case 3:
    strcpy(mon, "Apr");
    break;
  case 4:
    strcpy(mon, "May");
    break;
  case 5:
    strcpy(mon, "Jun");
    break;
  case 6:
    strcpy(mon, "Jul");
    break;
  case 7:
    strcpy(mon, "Aug");
    break;
  case 8:
    strcpy(mon, "Sep");
    break;
  case 9:
    strcpy(mon, "Oct");
    break;
  case 10:
    strcpy(mon, "Nov");
    break;
  case 11:
    strcpy(mon, "Dec");
    break;
  }
  Header header;
  header.name = calloc(1, strlen("Date:") + 1);
  strcpy(header.name, "Date:");
  header.value = calloc(1, (2 * (DAY_LEN + 1)) + (5 * (sizeof(int))) + 6);
  snprintf(header.value, (2 * (strlen(day) + 1)) + (5 * (sizeof(int)) + 6),
           "%s, %.2d %s %d %.2d:%.2d:%.2d GMT\r\n", day, time.tm_mday, mon,
           1900 + time.tm_year, time.tm_hour, time.tm_min, time.tm_sec);
  return header;
}
Header build_age(Request req) {
  Header header;
  header.name = calloc(1, strlen("Age:") + 1);
  strcpy(header.name, "Age:");
  header.value = calloc(1, SIG_FIGS + 5);
  gcvt(req.time_elapsed, SIG_FIGS, header.value);
  strcat(header.value, "\r\n");
  return header;
}
Header build_connection() {
  Header header;
  header.name = calloc(1, strlen("Connection:") + 1);
  strcpy(header.name, "Connection:");
  header.value = calloc(1, strlen("Closed\r\n") + 1);
  strcpy(header.value, "Closed\r\n");
  return header;
}
Header build_cookie(Request r) {
  Header header;
  header.name = calloc(1, strlen("Cookie: ") + 1);
  strcpy(header.name, "Cookie: ");
  header.value = calloc(1, 33);
  snprintf(header.value, 32, "%s\r\n", generate_cookie(time(NULL)));
  return header;
}
void build_entity_body(Response *rep, Request *req) {
  get_query(req, req->query, rep);
  rep->entity_body = encode_base64(rep->entity_body);
}
void send_response(Response *rep, FILE *out) {
  fprintf(out, "%s %s", rep->stat_line.stat_code, rep->stat_line.http_ver);
  if (strcmp(rep->stat_line.stat_code, "404 Not Found\r\n") == 0) {
    fprintf(out, "\r\n");
    fflush(out);
    return;
  }
  for (int i = 0; i < SERV_HEADER_COUNT; ++i) {
    fprintf(out, "%s", rep->headers[i].name);
    fprintf(out, "%s", rep->headers[i].value);
  }
  if (rep->entity_body != NULL) {
    fprintf(out, "%s", rep->entity_body);
  }
  fflush(out);
}
void destroy_response(Response *rep) {
  free(rep->stat_line.http_ver);
  rep->stat_line.http_ver = NULL;
  free(rep->stat_line.stat_code);
  rep->stat_line.stat_code = NULL;
  destroy_headers(rep->headers);
  free(rep->entity_body);
  rep->entity_body = NULL;
}
