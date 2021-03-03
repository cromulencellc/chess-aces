#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "HTTP_Request_Entry.h"
#define MAX_FILE_NAME 64
#define MAX_VER 16
#define MAX_METHOD_SIZE 8
#define MAX_RESOURCE_SIZE 256
#define MAX_DATE 256
#define MAX_RESULT 32
#define LOG_PATH "/data/log/"
#define LOG_FILE_NAME "HTTP.log"
void create_log() {
  char *path = malloc(strlen(LOG_PATH) + strlen(LOG_FILE_NAME) + 1);
  snprintf(path, MAX_FILE_NAME, "%s%s", LOG_PATH, LOG_FILE_NAME);
  path[strlen(LOG_PATH) + strlen(LOG_FILE_NAME)] = '\0';
  if (!access(path, F_OK)) {
    printf("The log file already exists\n");
    free(path);
    path = NULL;
    return;
  }
  FILE *fptr;
  if ((fptr = fopen(path, "w")) == NULL) {
    printf("There was a problem creating the HTTP log file\n");
    free(path);
    path = NULL;
    return;
  }
  free(path);
  path = NULL;
  fwrite("DATE\t", strlen("DATE\t"), 1, fptr);
  fwrite("METHOD\t", strlen("METHOD\t"), 1, fptr);
  fwrite("RESOURCE\t", strlen("RESOURCE\t"), 1, fptr);
  fwrite("HTTP_VER\t", strlen("HTTP_VER\t"), 1, fptr);
  fwrite("RESULT\n", strlen("RESULT\n"), 1, fptr);
  fclose(fptr);
}
HTTP_Request_Entry *create_entry(Response *response, Request *request) {
  HTTP_Request_Entry *entry = intialize_HTTP_Request();
  entry->method = calloc(1, MAX_METHOD_SIZE);
  strncpy(entry->method, request->method, MAX_METHOD_SIZE);
  entry->resource_accessed = calloc(1, MAX_RESOURCE_SIZE);
  strncpy(entry->resource_accessed, request->path, MAX_RESOURCE_SIZE);
  entry->http_version = calloc(1, MAX_VER);
  strncpy(entry->http_version, response->stat_line.http_ver, MAX_VER);
  time_t t = time(NULL);
  struct tm time = *gmtime(&t);
  char *day = calloc(1, 4);
  char *mon = calloc(1, 4);
  switch (time.tm_wday) {
  case 0:
    strcpy(day, "Sun");
    day[3] = '\0';
    break;
  case 1:
    strcpy(day, "Mon");
    day[3] = '\0';
    break;
  case 2:
    strcpy(day, "Tue");
    day[3] = '\0';
    break;
  case 3:
    strcpy(day, "Wed");
    day[3] = '\0';
    break;
  case 4:
    strcpy(day, "Thu");
    day[3] = '\0';
    break;
  case 5:
    strcpy(day, "Fri");
    day[3] = '\0';
    break;
  case 6:
    strcpy(day, "Sat");
    day[3] = '\0';
    break;
  }
  switch (time.tm_mon) {
  case 0:
    strcpy(mon, "Jan");
    mon[3] = '\0';
    break;
  case 1:
    strcpy(mon, "Feb");
    mon[3] = '\0';
    break;
  case 2:
    strcpy(mon, "Mar");
    mon[3] = '\0';
    break;
  case 3:
    strcpy(mon, "Apr");
    mon[3] = '\0';
    break;
  case 4:
    strcpy(mon, "May");
    mon[3] = '\0';
    break;
  case 5:
    strcpy(mon, "Jun");
    mon[3] = '\0';
    break;
  case 6:
    strcpy(mon, "Jul");
    mon[3] = '\0';
    break;
  case 7:
    strcpy(mon, "Aug");
    mon[3] = '\0';
    break;
  case 8:
    strcpy(mon, "Sep");
    mon[3] = '\0';
    break;
  case 9:
    strcpy(mon, "Oct");
    mon[3] = '\0';
    break;
  case 10:
    strcpy(mon, "Nov");
    mon[3] = '\0';
    break;
  case 11:
    strcpy(mon, "Dec");
    mon[3] = '\0';
    break;
  }
  entry->date = calloc(1, MAX_DATE);
  snprintf(entry->date, MAX_DATE, "%s %d %s %d %.2d:%.2d:%.2d GMT", day,
           time.tm_mday, mon, 1900 + time.tm_year, time.tm_hour, time.tm_min,
           time.tm_sec);
  free(day);
  day = NULL;
  free(mon);
  day = NULL;
  entry->stat_line_result = calloc(1, MAX_RESULT);
  strncpy(entry->stat_line_result, response->stat_line.stat_code, MAX_RESULT);
  return entry;
}
HTTP_Request_Entry *intialize_HTTP_Request() {
  HTTP_Request_Entry *http_req = malloc(sizeof(HTTP_Request_Entry));
  http_req->date = NULL;
  http_req->method = NULL;
  http_req->resource_accessed = NULL;
  http_req->http_version = NULL;
  http_req->stat_line_result = NULL;
  return http_req;
}
void record_entry(HTTP_Request_Entry *entry) {
  FILE *fptr;
  char *path = malloc(strlen(LOG_PATH) + strlen(LOG_FILE_NAME) + 1);
  snprintf(path, MAX_FILE_NAME, "%s%s", LOG_PATH, LOG_FILE_NAME);
  path[strlen(LOG_PATH) + strlen(LOG_FILE_NAME)] = '\0';
  if ((fptr = fopen(path, "a")) == NULL) {
    printf("HTTP Request log was not able to open for writing...\n");
    free(path);
    path = NULL;
    return;
  }
  fprintf(fptr, "%s\t", entry->date);
  fprintf(fptr, "%s\t", entry->method);
  fprintf(fptr, "%s\t", entry->resource_accessed);
  fprintf(fptr, "%s\t", entry->http_version);
  fprintf(fptr, "%s\n", entry->stat_line_result);
  fclose(fptr);
}
void print_HTTP_Request_Entry(HTTP_Request_Entry *entry) {
  printf("\nDATE: %s\n", entry->date);
  printf("METHOD: %s\n", entry->method);
  printf("RESOURCE: %s\n", entry->resource_accessed);
  printf("STAT_LINE_HTTP VER: %s\n", entry->http_version);
  printf("STAT_LINE_RESULT: %s\n\n", entry->stat_line_result);
}
void destroy_HTTP_Request_Entry(HTTP_Request_Entry *entry) {
  free(entry->date);
  entry->date = NULL;
  free(entry->method);
  entry->method = NULL;
  free(entry->resource_accessed);
  entry->resource_accessed = NULL;
  free(entry->http_version);
  entry->http_version = NULL;
  free(entry->stat_line_result);
  entry->stat_line_result = NULL;
  free(entry);
  entry = NULL;
}
