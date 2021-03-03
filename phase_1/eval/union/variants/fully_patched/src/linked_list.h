#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include "node.h"
typedef struct Linked_List {
  Node *head;
  size_t node_count;
} Linked_List;
Linked_List *create_linked_list();
void insert_to_list(Linked_List *l, Node *node);
Node *find_node(Linked_List *linked, Node *node);
void delete_node(Linked_List *linked, Node *del_node);
void print_linked_list(Linked_List *l);
void destroy_linked_list(Linked_List *ll);
#endif
