#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "b_tree.h"
#include "cookie_generation.h"
#include "header.h"
#include "post.h"
#include "process.h"
#include "request.h"
#include "response.h"
#define SERV_HEADER_COUNT 4
#define MAX_HEADER_COUNT 32
#define MAX_FILE_SIZE 4096
#define DAY_LEN 3
#define SIG_FIGS 6
#define ROOT_DIR "/data"
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
  Stat_Line stat_line = process_req_method(req);
  return stat_line;
}
Response assemble_response(Request req, FILE *out, RoleTable *rt) {
  Response response;
  response.exit = false;
  response.stat_line = build_status_line(req);
  response.headers = calloc(MAX_HEADER_COUNT, sizeof(Header));
  response.headers[0] = build_date();
  response.headers[1] = build_age(req);
  response.headers[2] = build_connection();
  response.headers[3] = build_cookie(req, rt);
  build_entity_body(&response, &req);
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
Header build_cookie(Request r, RoleTable *rt) {
  Header header, dummy;
  int role_number = -1;
  header.name = calloc(1, strlen("Set-Cookie:") + 1);
  strncpy(header.name, "Set-Cookie:", strlen("Set-Cookie:"));
  dummy = find_role_tag(r.headers);
  header.value = calloc(1, 39);
  if (dummy.value != NULL &&
      strncmp(dummy.value, " ANONYMOUS", strlen(" ANONYMOUS")) == 0) {
    role_number = atoi(rt->entries[0].number);
    if (role_number >= 0 || role_number <= 2) {
      snprintf(header.value, 34, " %s", rt->entries[role_number].cookie);
      header.value[34] = '\0';
      strcat(header.value, "\r\n\r\n");
    }
  } else if (dummy.value != NULL &&
             strncmp(dummy.value, " USER", strlen(" USER")) == 0) {
    role_number = atoi(rt->entries[1].number);
    if (role_number >= 0 || role_number <= 2) {
      snprintf(header.value, 34, " %s", rt->entries[role_number].cookie);
      header.value[34] = '\0';
      strcat(header.value, "\r\n\r\n");
    }
  } else if (dummy.value != NULL &&
             strncmp(dummy.value, " ADMIN", strlen(" ADMIN")) == 0) {
    role_number = atoi(rt->entries[0].number);
    if (role_number >= 0 || role_number <= 2) {
      snprintf(header.value, 34, "%s", rt->entries[role_number].cookie);
      header.value[34] = '\0';
      strcat(header.value, "\r\n\r\n");
    }
  } else {
    strncpy(header.value, "0\r\n\r\n", 37);
  }
  return header;
}
void build_entity_body(Response *rep, Request *req) {
  FILE *fstream = NULL;
  if (strcmp(req->method, "GET") == 0) {
    if (req->query_flag) {
      if (strcmp(req->path, "/wiki") != 0) {
        rep->entity_body = malloc(2);
        strncpy(rep->entity_body, " ", 1);
        rep->entity_body[1] = '\0';
        return;
      } else {
        Search_Result *result_list = start_search(req->query, req->path);
        if (result_list == NULL) {
          return;
        }
        int i;
        for (i = 0; i < result_list->item_count; ++i) {
          if (i == 0) {
            rep->entity_body = malloc(strlen(result_list->final_result[i]) + 2);
            strncpy(rep->entity_body, result_list->final_result[i],
                    strlen(result_list->final_result[i]));
            rep->entity_body[strlen(result_list->final_result[i])] = '\0';
            strcat(rep->entity_body, "\n");
            continue;
          }
          rep->entity_body = realloc(
              rep->entity_body, strlen(rep->entity_body) +
                                    strlen(result_list->final_result[i]) + 2);
          strcat(rep->entity_body, result_list->final_result[i]);
          rep->entity_body[strlen(rep->entity_body)] = '\0';
          strcat(rep->entity_body, "\n");
        }
        return;
      }
    }
    char *path_cpy = strdup(req->path);
    char *stream_path = calloc(1, strlen(ROOT_DIR) + strlen(path_cpy) + 1);
    snprintf(stream_path, strlen(ROOT_DIR) + strlen(path_cpy) + 1, "%s%s",
             ROOT_DIR, path_cpy);
    printf("PATH: %s\n", stream_path);
    fstream = fopen(stream_path, "r");
    if (fstream == NULL) {
      rep->entity_body = NULL;
      printf("fstream broke\n");
      return;
    }
    fseek(fstream, 0L, SEEK_END);
    int size = ftell(fstream);
    rewind(fstream);
    rep->entity_body = calloc(1, size + 1);
    fread(rep->entity_body, size, 1, fstream);
    fclose(fstream);
  } else {
    printf("The request as it was made is not supported...\n");
    rep->entity_body = NULL;
    return;
  }
}
void send_response(Response *rep, FILE *out) {
  fprintf(out, "%s %s", rep->stat_line.http_ver, rep->stat_line.stat_code);
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
