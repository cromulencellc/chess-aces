#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include "rust.h"
const static struct object list_object = {(object_delete_f)list_delete,
                                          (object_copy_f)list_copy,
                                          (object_cmp_f)object_not_comparable};
const struct object *object_type_list = &list_object;
struct list *list_create(const struct object *type) {
  struct list *list = (struct list *)unwrap_null(malloc(sizeof(struct list)));
  list->object = &list_object;
  list->type = type;
  list->head = NULL;
  list->tail = NULL;
  list->len = 0;
  return list;
}
void list_delete(struct list *list) {
  struct list_it *it;
  struct list_it *next;
  for (it = list->head; it != NULL; it = next) {
    next = it->next;
    object_delete(it->object);
    free(it);
  }
  free(list);
}
void list_delete_shallow(struct list *list) {
  struct list_it *it;
  struct list_it *next;
  for (it = list->head; it != NULL; it = next) {
    next = it->next;
    free(it);
  }
  free(list);
}
struct list *list_copy(const struct list *list) {
  struct list *new_list = list_create(list->type);
  const struct list_it *it;
  for (it = list->head; it != NULL; it = it->next) {
    list_append(new_list, object_copy(it->object));
  }
  return new_list;
}
void list_append(struct list *list, void *object) {
  object_type_verify(object, list->type);
  struct list_it *it = (struct list_it *)malloc(sizeof(struct list_it));
  it->next = NULL;
  it->prev = NULL;
  it->object = object;
  if (list->tail == NULL) {
    list->head = it;
    list->tail = it;
  } else {
    list->tail->next = it;
    it->prev = list->tail;
    list->tail = it;
  }
  list->len++;
}
void list_prepend(struct list *list, void *object) {
  object_type_verify(object, list->type);
  struct list_it *it = (struct list_it *)malloc(sizeof(struct list_it));
  it->next = NULL;
  it->prev = NULL;
  it->object = object;
  if (list->head == NULL) {
    list->head = it;
    list->tail = it;
  } else {
    list->head->prev = it;
    it->next = list->head;
    list->head = it;
  }
  list->len++;
}
struct list *list_split(struct list *list, unsigned int offset) {
  unsigned int i;
  struct list_it *it = list->head;
  for (i = 0; i < offset; i++) {
    if (it == NULL) {
      return NULL;
    }
    it = it->next;
  }
  if (it == NULL) {
    return NULL;
  }
  struct list *new_list = list_create(list->type);
  new_list->head = it;
  new_list->tail = list->tail;
  new_list->len = list->len - offset;
  list->tail = it->prev;
  if (list->head == it) {
    list->head = NULL;
  }
  list->len = offset;
  if (it->prev != NULL) {
    it->prev->next = NULL;
  }
  it->prev = NULL;
  return new_list;
}
struct list *list_sort_(struct list *list,
                        int (*cmp)(const void *, const void *)) {
  if (list->len == 0) {
    return list;
  } else if (list->len == 1) {
    return list;
  }
  struct list *rhs = list_split(list, (list->len + 1) / 2);
  struct list *lhs = list_sort_(list, cmp);
  rhs = list_sort_(rhs, cmp);
  struct list *new_list = list_create(lhs->type);
  struct list_it *lhs_it = lhs->head;
  struct list_it *rhs_it = rhs->head;
  while ((lhs_it != NULL) || (rhs_it != NULL)) {
    if (lhs_it == NULL) {
      list_append(new_list, rhs_it->object);
      rhs_it = rhs_it->next;
    } else if (rhs_it == NULL) {
      list_append(new_list, lhs_it->object);
      lhs_it = lhs_it->next;
    } else {
      if (cmp(lhs_it->object, rhs_it->object) < 0) {
        list_append(new_list, lhs_it->object);
        lhs_it = lhs_it->next;
      } else {
        list_append(new_list, rhs_it->object);
        rhs_it = rhs_it->next;
      }
    }
  }
  list_delete_shallow(lhs);
  list_delete_shallow(rhs);
  return new_list;
}
void list_sort(struct list **list) {
  if ((*list)->len > 0) {
    struct object *object = *((struct object **)(*list)->head->object);
    *list = list_sort_(*list, object->cmp);
  }
}
void list_sort_custom(struct list **list,
                      int (*cmp)(const void *, const void *)) {
  *list = list_sort_(*list, cmp);
}
void list_dedup(struct list *list) {
  if (list->len == 0)
    return;
  struct list_it *it = list->head;
  while ((it != NULL) && (it->next != NULL)) {
    if (object_cmp(it->object, it->next->object) == 0) {
      struct list_it *next = it->next;
      it->next = next->next;
      if (next->next != NULL) {
        next->next->prev = it;
      } else {
        list->tail = it;
      }
      object_delete(next->object);
      free(next);
      list->len--;
    }
    it = it->next;
  }
}
void list_dedup_shallow(struct list *list) {
  if (list->len == 0)
    return;
  struct list_it *it = list->head;
  while ((it != NULL) && (it->next != NULL)) {
    if (object_cmp(it->object, it->next->object) == 0) {
      struct list_it *next = it->next;
      it->next = next->next;
      if (next->next != NULL) {
        next->next->prev = it;
      } else {
        list->tail = it;
      }
      free(next);
      list->len--;
    }
    it = it->next;
  }
}
void list_join(struct list *list, struct list *rhs) {
  object_type_verify(&list->type, rhs->type);
  struct list_it *it;
  for (it = rhs->head; it != NULL; it = it->next) {
    list_append(list, it->object);
  }
  list_delete_shallow(rhs);
}
struct list *list_map(struct list *list, const struct object *new_type,
                      void *(*f)(void *)) {
  struct list *new_list = list_create(new_type);
  struct list_it *it;
  for (it = list->head; it != NULL; it = it->next) {
    list_append(new_list, f(it->object));
  }
  return new_list;
}