#include <stdio.h>
#include <stdlib.h>
#include "linked_list.h"
Linked_List *create_linked_list() {
  Linked_List *new_list = calloc(1, sizeof(Linked_List));
  new_list->head = NULL;
  new_list->node_count = 0;
  return new_list;
}
void insert_to_list(Linked_List *l, Node *node) {
  if (l->head == NULL) {
    l->head = node;
    node->prev = NULL;
    node->next = NULL;
    l->node_count++;
  } else {
    Node *holder = l->head;
    while (holder->next != NULL) {
      holder = holder->next;
    }
    node->prev = holder;
    holder->next = node;
    node->next = NULL;
    l->node_count++;
  }
}
Node *find_node(Linked_List *linked, Node *node) {
  Node *found_node = NULL;
  if (linked == NULL) {
    fprintf(stderr, "[FIND_NODE] The linked_list provided is not allocated\n");
    return found_node;
  }
  if (linked->head == NULL) {
    fprintf(stderr,
            "[FIND_NODE]There is nothing in this linked list to find\n");
    return found_node;
  }
  Node *travel = linked->head;
  while (travel != NULL) {
    if (travel->data == node->data) {
      found_node = travel;
      return found_node;
    }
    travel = travel->next;
  }
  if (travel == NULL) {
    return NULL;
  }
  return found_node;
}
void delete_node(Linked_List *linked, Node *del_node) {
  if (linked == NULL) {
    fprintf(stderr,
            "[DELETE_NODE] The linked_list provided is not allocated\n");
    return;
  }
  if (linked->head == NULL) {
    fprintf(stderr,
            "[DELETE_NODE]There is nothing in this linked list to delete\n");
    return;
  }
  if (linked->node_count == 1) {
    linked->head = NULL;
    destroy_node(del_node);
    return;
  }
  if (linked->node_count == 2) {
    if (linked->head == del_node) {
      Node *tmp = linked->head;
      linked->head = linked->head->next;
      destroy_node(tmp);
      return;
    }
    if (linked->head->next != NULL) {
      destroy_node(linked->head->next);
      linked->head->next = NULL;
    } else {
      fprintf(stderr, "[DELETE_NODE] There is a problem with the connection of "
                      "the nodes\n");
      return;
    }
  }
  if (linked->node_count > 2) {
    if (del_node->prev == NULL) {
      linked->head = del_node->next;
      destroy_node(del_node);
      return;
    }
    if (del_node->next == NULL) {
      del_node->prev = NULL;
      destroy_node(del_node);
      return;
    }
    if (del_node->prev != NULL && del_node->next != NULL) {
      if (del_node->prev->next != NULL && del_node->next->prev != NULL) {
        del_node->prev->next = del_node->next;
        del_node->next->prev = del_node->prev;
        destroy_node(del_node);
        return;
      }
    } else {
      fprintf(stderr, "[DELETE_NODE] There is a problem with the connection of "
                      "the nodes\n");
      return;
    }
  }
}
void print_linked_list(Linked_List *l) {
  Node *holder;
  holder = l->head;
  while (holder != NULL) {
    printf("%p\n", holder);
    holder = holder->next;
  }
  printf("%p\n\n", holder);
}
void destroy_linked_list(Linked_List *ll) {
  Node *traveler = NULL;
  while (ll->head != NULL) {
    traveler = ll->head;
    ll->head = ll->head->next;
    destroy_node(traveler);
  }
  free(ll);
  ll = NULL;
}
