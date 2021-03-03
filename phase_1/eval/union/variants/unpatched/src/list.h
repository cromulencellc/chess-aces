#ifndef LIST_H
#define LIST_H
#include "entry.h"
#include "linked_list.h"
typedef struct List {
  char *name;
  char *path;
  Linked_List *entries;
} List;
List *create_list(char *session_path, char *name);
void add_entry(List *list, char *task);
void edit_list(List *list, char *new_name);
Entry *find_entry(List *list, char *task);
void delete_list_entry(List *list, char *task);
void print_list(List *list);
void destroy_list(List *list);
#endif
