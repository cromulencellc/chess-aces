#include "list.h"
#include <stdio.h>
#include <stdlib.h>
struct list *list_create() {
  struct list *list = (struct list *)malloc(sizeof(struct list));
  list->head = NULL;
  list->tail = NULL;
  return list;
}
void list_delete(struct list *list, void (*del)(void *)) {
  struct list_it *it;
  struct list_it *next;
  for (it = list->head; it != NULL; it = next) {
    next = it->next;
    del(it->data);
    free(it);
  }
  free(list);
}
void list_append(struct list *list, void *data) {
  struct list_it *it = (struct list_it *)malloc(sizeof(struct list_it));
  it->next = NULL;
  it->prev = NULL;
  it->data = data;
  if (list->tail == NULL) {
    list->head = it;
    list->tail = it;
  } else {
    list->tail->next = it;
    it->prev = list->tail;
    list->tail = it;
  }
}
void list_prepend(struct list *list, void *data) {
  struct list_it *it = (struct list_it *)malloc(sizeof(struct list_it));
  it->next = NULL;
  it->prev = NULL;
  it->data = data;
  if (list->head == NULL) {
    list->head = it;
    list->tail = it;
  } else {
    list->head->prev = it;
    it->next = list->head;
    list->head = it;
  }
}