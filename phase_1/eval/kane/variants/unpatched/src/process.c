#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "request.h"
#include "response.h"
#include "response_constants.h"
#define ROOT_DIR "/data"
Stat_Line process_req_method(Request req) {
  Stat_Line stat_line;
  stat_line.http_ver = calloc(1, strlen(HTTP_VER) + 1);
  strncpy(stat_line.http_ver, HTTP_VER, strlen(HTTP_VER));
  for (int i = 0; i < UNSUPPED_METHOD_COUNT; ++i) {
    if (strcmp(req.method, NOT_SUPPED_METHODS[i]) == 0) {
      stat_line.stat_code = calloc(1, strlen(METHOD_NOT_ALLOWED) + 1);
      strncpy(stat_line.stat_code, METHOD_NOT_ALLOWED,
              strlen(METHOD_NOT_ALLOWED));
      return stat_line;
    } else if (i == UNSUPPED_METHOD_COUNT - 1) {
      break;
    }
  }
  if (strcmp(req.method, "GET") == 0) {
    char *path_cpy = strdup(req.path);
    char *rsrc_path = calloc(1, strlen(ROOT_DIR) + strlen(req.path) + 1);
    snprintf(rsrc_path, strlen(ROOT_DIR) + strlen(req.path) + 1, "%s%s",
             ROOT_DIR, path_cpy);
    if (access(rsrc_path, F_OK) != 0) {
      printf("Resource not found\n");
      stat_line.stat_code = calloc(1, strlen(NOT_FOUND) + 1);
      strncpy(stat_line.stat_code, NOT_FOUND, strlen(NOT_FOUND));
      free(rsrc_path);
      rsrc_path = NULL;
      return stat_line;
    } else {
      printf("File check passed\n");
      free(rsrc_path);
      rsrc_path = NULL;
    }
  } else if (strcmp(req.method, "POST") == 0) {
  } else {
    stat_line.stat_code = calloc(1, strlen(METHOD_NOT_ALLOWED) + 1);
    strncpy(stat_line.stat_code, METHOD_NOT_ALLOWED,
            strlen(METHOD_NOT_ALLOWED));
    return stat_line;
  }
  stat_line.stat_code = calloc(1, strlen(OK) + 1);
  strncpy(stat_line.stat_code, OK, strlen(OK));
  return stat_line;
}
