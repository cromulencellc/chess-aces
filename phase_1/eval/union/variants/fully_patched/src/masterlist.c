#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "masterlist.h"
#define SESSION_PATH "/data/sessions/"
#define LOCAL_MLIST "/masterlist/mlist.log"
#define MAX_PATH 2048
#define MAX_LIST_NAME 64
MasterList *create_mlist(char *session_path) {
  MasterList *mlist = calloc(1, sizeof(MasterList));
  mlist->lists = create_linked_list();
  char *mpath = calloc(1, MAX_PATH + 1);
  snprintf(mpath, MAX_PATH, "%s%s", session_path, LOCAL_MLIST);
  printf("Masterlist Path: %s\n", mpath);
  if (access(mpath, F_OK) != 0) {
    FILE *mlptr;
    if ((mlptr = fopen(mpath, "w")) == NULL) {
      fprintf(stderr, "[CREATE_MLIST] Failed to open mlist.log file\n");
      return NULL;
    }
    fclose(mlptr);
  }
  free(mpath);
  mpath = NULL;
  return mlist;
}
void add_to_master(MasterList *mlist, List *list) {
  if (mlist == NULL) {
    fprintf(stderr, "[ADD_TO_MASTER] Master list provided empty");
    return;
  }
  if (mlist->lists == NULL) {
    fprintf(stderr, "[ADD_TO_MASTER]No linked list in masterlist");
    return;
  }
  if (list == NULL) {
    fprintf(stderr,
            "[ADD_TO_MASTER] the list that you provided is not allocated");
    return;
  }
  Node *node = create_node();
  node->data = list;
  insert_to_list(mlist->lists, node);
}
List *find_list(MasterList *mlist, char *listname) {
  List *found_list = NULL;
  if (mlist == NULL) {
    fprintf(stderr, "[FIND_LIST] Master list provided empty");
    return NULL;
  }
  if (mlist->lists == NULL) {
    fprintf(stderr, "[FIND_LIST]No linked list in masterlist");
    return NULL;
  }
  if (listname == NULL) {
    fprintf(stderr,
            "[FIND_LIST] the listname that you provided is not allocated");
    return NULL;
  }
  Node *traveler = mlist->lists->head;
  List *holder;
  while (traveler != NULL) {
    holder = traveler->data;
    if (strncmp(holder->name, listname, MAX_LIST_NAME) == 0) {
      found_list = holder;
      return found_list;
    } else {
      traveler = traveler->next;
    }
  }
  return found_list;
}
void delete_from_master(MasterList *mlist, char *listname) {
  if (mlist == NULL) {
    fprintf(stderr, "[FIND_LIST] Master list provided empty");
    return;
  }
  if (mlist->lists == NULL) {
    fprintf(stderr, "[FIND_LIST]No linked list in masterlist");
    return;
  }
  if (listname == NULL) {
    fprintf(stderr,
            "[FIND_LIST] the listname that you provided is not allocated");
    return;
  }
  Node *traveler = mlist->lists->head;
  List *holder;
  while (traveler != NULL) {
    holder = traveler->data;
    if (strncmp(holder->name, listname, MAX_LIST_NAME) == 0) {
      delete_node(mlist->lists, traveler);
      return;
    } else {
      traveler = traveler->next;
    }
  }
}
void print_master(MasterList *mlist) {
  printf("----MASTERLIST----\n");
  if (mlist == NULL) {
    fprintf(stderr, "[PRINT_MASTER] The masterlist provided is empty\n");
    return;
  }
  if (mlist->lists == NULL) {
    fprintf(stderr, "[PRINT_MASTER] The masterlist->lists field is empty\n");
    return;
  }
  Node *traveler = mlist->lists->head;
  List *holder;
  while (traveler != NULL) {
    holder = traveler->data;
    print_list(holder);
    if (traveler->next == NULL) {
      break;
    } else {
      traveler = traveler->next;
    }
  }
}
void destroy_mlist(MasterList *mlist) {
  if (mlist->lists != NULL) {
    destroy_linked_list(mlist->lists);
  }
  if (mlist != NULL) {
    free(mlist);
    mlist = NULL;
  }
}
