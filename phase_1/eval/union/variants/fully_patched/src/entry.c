#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entry.h"
#define MAX_TASK_LENGTH 512
Entry *create_entry(char *task) {
  if (task == NULL) {
    fprintf(stderr,
            "[CREATE_ENTRY] The task you provided does not contain a value\n");
    return NULL;
  }
  Entry *new_entry = calloc(1, sizeof(Entry));
  new_entry->task = calloc(1, MAX_TASK_LENGTH + 1);
  strncpy(new_entry->task, task, MAX_TASK_LENGTH);
  new_entry->task_len = strlen(new_entry->task);
  new_entry->completed = false;
  return new_entry;
}
void print_entry(Entry *entry) {
  printf("task: %s\n", entry->task);
  printf("task_len: %ld\n", entry->task_len);
  if (entry->completed) {
    printf("completed?: TRUE\n\n\n");
  } else {
    printf("completed?: FALSE\n\n\n");
  }
}
void edit_entry_task(Entry *entry, char *change_to) {
  if (entry == NULL) {
    fprintf(stderr, "[ENTRY_EDIT] There is no allocated entry\n");
    return;
  }
  if (entry->task == NULL) {
    fprintf(stderr, "[ENTRY_EDIT] The task is mangled cannot edit\n");
    return;
  }
  if (change_to == NULL) {
    fprintf(stderr, "[ENTRY_EDIT] No text was provided for the change\n");
    return;
  }
  memset(entry->task, 0, MAX_TASK_LENGTH);
  strncpy(entry->task, change_to, MAX_TASK_LENGTH);
  entry->task_len = strlen(entry->task);
}
void set_to_complete(Entry *entry) {
  if (entry == NULL) {
    fprintf(stderr, "[SET_TO_COMPLETE] There is no allocated entry\n");
    return;
  }
  entry->completed = true;
}
void set_to_incomplete(Entry *entry) {
  if (entry == NULL) {
    fprintf(stderr, "[SET_TO_INCOMPLETE] There is no allocated entry\n");
    return;
  }
  entry->completed = false;
}
void delete_entry(Entry *entry) {
  if (entry == NULL) {
    fprintf(stderr, "[DELETE_ENTRY]The entry is not allocated\n");
  }
  if (entry->task != NULL) {
    memset(entry->task, 0, MAX_TASK_LENGTH);
    free(entry->task);
    entry->task = NULL;
  }
  if (entry != NULL) {
    free(entry);
    entry = NULL;
  }
}
