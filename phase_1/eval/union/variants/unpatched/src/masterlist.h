#ifndef MASTERLIST_H
#define MASTERLIST_H
#include "linked_list.h"
#include "list.h"
typedef struct MasterList {
  Linked_List *lists;
} MasterList;
MasterList *create_mlist(char *session_path);
void add_to_master(MasterList *mlist, List *list);
List *find_list(MasterList *mlist, char *listname);
void print_master(MasterList *mlist);
void delete_from_master(MasterList *mlist, char *listname);
void destroy_mlist();
#endif
