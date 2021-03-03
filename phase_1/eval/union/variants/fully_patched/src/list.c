#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entry.h"
#include "list.h"
#define MAX_LIST_NAME 64
#define MAX_ENTRY_SIZE 512
#define MAX_PATH 2048
List *create_list(char *session_path, char *name) {
  List *list = calloc(1, sizeof(List));
  list->name = calloc(1, MAX_LIST_NAME + 1);
  strncpy(list->name, name, MAX_LIST_NAME);
  list->path = calloc(1, MAX_PATH + 1);
  snprintf(list->path, MAX_PATH, "%s/%s/%s.txt", session_path, "lists",
           list->name);
  list->entries = create_linked_list();
  return list;
}
void add_entry(List *list, char *task) {
  if (list == NULL) {
    fprintf(stderr, "[ADD_ENTRY] List struct is not allocated\n");
    return;
  }
  if (list->entries == NULL) {
    fprintf(stderr, "[ADD_ENTRY] Entires ll not allocated\n");
    return;
  }
  if (task == NULL) {
    fprintf(stderr, "[ADD_ENTRY] The task given is empty\n");
    return;
  }
  Node *node = create_node();
  node->data = create_entry(task);
  insert_to_list(list->entries, node);
}
void edit_list(List *list, char *new_name) {
  if (list == NULL) {
    fprintf(stderr, "[ADD_ENTRY] List struct is not allocated\n");
    return;
  }
  if (new_name == NULL) {
    fprintf(stderr, "[ADD_ENTRY] The new name given is empty\n");
    return;
  }
  memset(list->name, 0, MAX_LIST_NAME);
  strncpy(list->name, new_name, MAX_LIST_NAME);
}
Entry *find_entry(List *list, char *task) {
  Entry *found_entry = NULL;
  if (list == NULL) {
    fprintf(stderr, "[FIND_ENTRY] List struct is not allocated\n");
    return found_entry;
  }
  if (list->entries == NULL) {
    fprintf(stderr, "[FIND_ENTRY] Entires ll not allocated\n");
    return found_entry;
  }
  if (task == NULL) {
    fprintf(stderr, "[FIND_ENTRY] The task given is empty\n");
    return found_entry;
  }
  Node *traveler = list->entries->head;
  Entry *holder;
  while (traveler != NULL) {
    holder = traveler->data;
    if (strncmp(holder->task, task, MAX_ENTRY_SIZE) == 0) {
      found_entry = holder;
      return found_entry;
    } else {
      traveler = traveler->next;
    }
  }
  return found_entry;
}
void print_list(List *list) {
  if (list == NULL) {
    fprintf(stderr, "[PRINT_LIST] The list provided is empty\n");
    return;
  }
  if (list->name == NULL) {
    fprintf(stderr, "[PRINT_LIST] The list->name field is empty\n");
    return;
  }
  if (list->entries == NULL) {
    fprintf(stderr, "[PRINT_LIST] There are no entries in the linked list");
    return;
  }
  printf("----LIST NAME----%s\n", list->name);
  Node *traveler = list->entries->head;
  Entry *holder;
  while (traveler != NULL) {
    holder = traveler->data;
    printf("Entry: %s\n", holder->task);
    if (traveler->next == NULL) {
      break;
    } else {
      traveler = traveler->next;
    }
  }
}
void delete_list_entry(List *list, char *task) {
  if (list == NULL) {
    fprintf(stderr, "[DELETE_ENTRY] The list provided is empty\n");
    return;
  }
  if (list->entries == NULL) {
    fprintf(stderr,
            "[DELETE_ENTRY] There are no entries to delete in the list\n");
    return;
  }
  if (task == NULL) {
    fprintf(stderr, "[DELETE_ENTRY] The task provided is null");
    return;
  }
  Node *traveler = list->entries->head;
  Entry *holder;
  while (traveler != NULL) {
    holder = traveler->data;
    if (strncmp(holder->task, task, MAX_ENTRY_SIZE) == 0) {
      delete_node(list->entries, traveler->data);
      return;
    } else {
      traveler = traveler->next;
    }
  }
}
void destroy_list(List *list) {
  if (list->name != NULL) {
    free(list->name);
    list->name = NULL;
  }
  if (list->path != NULL) {
    free(list->path);
    list->path = NULL;
  }
  if (list->entries != NULL) {
    destroy_linked_list(list->entries);
  }
  if (list != NULL) {
    free(list);
    list = NULL;
  }
}
