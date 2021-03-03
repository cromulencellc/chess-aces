#define _GNU_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "get.h"
void get_cand_path(char *path) {
  printf("PATH: %s\n", path);
  if (getenv("ROOT_DIR") == NULL) {
    fprintf(stderr, "cand_dir enviorment var problem\n\n");
  }
  char *cand_dir = strdup(getenv("ROOT_DIR"));
  if ((cand_dir = realloc(cand_dir, strlen(cand_dir) + strlen(path) + 2)) ==
      NULL) {
    printf("Error reallocing cand_dir\n");
  }
  strcat(cand_dir, path);
  strncpy(path, cand_dir, strlen(cand_dir) + 1);
  path[strlen(path)] = '\0';
  free(cand_dir);
}
