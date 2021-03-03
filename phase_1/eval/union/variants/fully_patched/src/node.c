#include <stdio.h>
#include <stdlib.h>
#include "node.h"
Node *create_node() {
  Node *new_node = calloc(1, sizeof(Node));
  new_node->data = NULL;
  new_node->prev = NULL;
  new_node->next = NULL;
  return new_node;
}
void destroy_node(Node *node) {
  node->prev = NULL;
  node->next = NULL;
  node->data = NULL;
  free(node);
  node = NULL;
}
