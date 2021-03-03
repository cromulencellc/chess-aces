#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "line_state_table.h"
#include "user_list.h"
#define PATH "/data/user_list/"
#define FILENAME "ulist.log"
#define MAX_CRED_LEN 64
#define MAX_LIST_SIZE 4096
LineStateTable *initalize_ls_table() {
  char *canon_path = canonicalize_file_name(PATH);
  LineStateTable *lst = malloc(sizeof(LineStateTable));
  if (canon_path == NULL) {
    fprintf(stderr, "There was an error canonicalizing the path\n");
    free(canon_path);
    canon_path = NULL;
    return lst;
  } else {
    fprintf(stderr, "RETRIEVE_USER_LIST: Canon path exists\n");
  }
  char *file_list = calloc(1, strlen(canon_path) + strlen(FILENAME) + 2);
  snprintf(file_list, strlen(canon_path) + strlen(FILENAME) + 2, "%s/%s",
           canon_path, FILENAME);
  FILE *fptr = fopen(file_list, "r");
  if (fptr == NULL) {
    fprintf(stderr, "INTIALIZE_LS_TABLE: Failed to open file for reading\n");
    free(canon_path);
    canon_path = NULL;
    free(file_list);
    file_list = NULL;
    return lst;
  }
  if (access(file_list, F_OK) != 0) {
    fprintf(stderr, "INTIALIZE_LS_TABLE: The file did not exist\n");
    FILE *fptr = fopen(file_list, "w");
    free(canon_path);
    canon_path = NULL;
    free(file_list);
    file_list = NULL;
    fclose(fptr);
    return lst;
  } else {
    int number_of_lines = 0;
    char *buffer = calloc(1, MAX_CRED_LEN + 1);
    while (fgets(buffer, MAX_CRED_LEN, fptr) != NULL) {
      number_of_lines++;
    }
    lst->entry_count = number_of_lines;
    printf("Number of Lines: %d\n", number_of_lines);
    if (number_of_lines > 0) {
      lst->line_offset = calloc(number_of_lines, sizeof(int));
      lst->state = calloc(number_of_lines, sizeof(int));
      lst->line_offset[0] = 0;
      lst->state[0] = 1;
    }
    rewind(fptr);
    int i, number_of_bytes = 0;
    for (i = 1; i < lst->entry_count; ++i) {
      while (fgetc(fptr) != '\n') {
        number_of_bytes++;
      }
      lst->line_offset[i] = (i + number_of_bytes);
      lst->state[i] = 1;
    }
    for (i = 0; i < lst->entry_count; ++i) {
      fseek(fptr, lst->line_offset[i], SEEK_SET);
      fgets(buffer, MAX_CRED_LEN, fptr);
      rewind(fptr);
    }
    free(buffer);
    buffer = NULL;
    free(canon_path);
    canon_path = NULL;
    free(file_list);
    file_list = NULL;
    fclose(fptr);
  }
  return lst;
}
void mark_for_delete(LineStateTable *lstable) {}
void print_lstable(LineStateTable *lstable) {
  char *canon_path = canonicalize_file_name(PATH);
  if (canon_path == NULL) {
    fprintf(stderr, "There was an error canonicalizing the path\n");
    free(canon_path);
    canon_path = NULL;
    return;
  } else {
    fprintf(stderr, "RETRIEVE_USER_LIST: Canon path exists\n");
  }
  char *file_list = calloc(1, strlen(canon_path) + strlen(FILENAME) + 2);
  snprintf(file_list, strlen(canon_path) + strlen(FILENAME) + 2, "%s/%s",
           canon_path, FILENAME);
  FILE *fptr = fopen(file_list, "r");
  if (fptr == NULL) {
    fprintf(stderr, "There is a problem opening the file for reading\n");
    return;
  }
  char *buffer = calloc(1, MAX_CRED_LEN);
  int i;
  for (i = 0; i < lstable->entry_count; ++i) {
    printf("Line Offset: %d\n", lstable->line_offset[i]);
    printf("State: %d\n", lstable->state[i]);
    fseek(fptr, lstable->line_offset[i], SEEK_SET);
    fgets(buffer, MAX_CRED_LEN - 1, fptr);
    rewind(fptr);
    printf("Buffer: %s\n\n", buffer);
  }
  free(buffer);
  buffer = NULL;
  free(canon_path);
  canon_path = NULL;
  free(file_list);
  file_list = NULL;
  fclose(fptr);
}
void destroy_lstable(LineStateTable *lstable) {
  free(lstable->line_offset);
  lstable->line_offset = NULL;
  free(lstable->state);
  lstable->state = NULL;
  free(lstable);
  lstable = NULL;
}
