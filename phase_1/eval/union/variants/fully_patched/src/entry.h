#ifndef ENTRY_H
#define ENTRY_H
#include <stdbool.h>
typedef struct Entry {
  char *task;
  size_t task_len;
  bool completed;
} Entry;
Entry *create_entry(char *task);
void print_entry(Entry *entry);
void edit_entry_task(Entry *entry, char *change_to);
void delete_entry(Entry *entry);
void set_to_complete(Entry *entry);
void set_to_incomplete(Entry *entry);
#endif
