#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "get_query.h"
#define STAT_PATH "/data/stats/stat.log"
#define ACHIEVE_TRACKER "achieve_tracker"
#define MAX_TRACKER 2048
void get_query(Request *req, char *query, Response *resp) {
  if (query == NULL) {
    fprintf(stderr, "[GET_QUERY] The query string was empty\n");
    return;
  }
  char *req_cpy = strdup(query);
  char *holder = NULL;
  if ((holder = strchr(req_cpy, '=')) == NULL) {
    fprintf(stderr, "[GET_QUERY] Not able to tok correctly\n");
    free(req_cpy);
    req_cpy = NULL;
    return;
  }
  holder += 1;
  printf("Action: %s\n", holder);
  if (strncmp(holder, ACHIEVE_TRACKER, strlen(ACHIEVE_TRACKER)) == 0) {
    FILE *rfptr;
    if ((rfptr = fopen(STAT_PATH, "r")) == NULL) {
      fprintf(stderr, "[GET_QUERY] There is no stat.log file to get\n");
      req->error_number = 4;
      free(req_cpy);
      req_cpy = NULL;
      return;
    }
    char buffer[MAX_TRACKER + 1];
    fread(buffer, MAX_TRACKER, 1, rfptr);
    buffer[MAX_TRACKER] = '\0';
    resp->entity_body = calloc(1, MAX_TRACKER + 1);
    strncpy(resp->entity_body, buffer, MAX_TRACKER);
    printf("CHECK %s\n", resp->entity_body);
    fclose(rfptr);
    rfptr = NULL;
  }
  free(req_cpy);
  req_cpy = NULL;
}
