#ifndef list_HEADER
#define list_HEADER

#include "object.h"

/* This list takes objects */

extern const struct object * object_type_list;

struct list_it {
    void * object;
    struct list_it * next;
    struct list_it * prev;
};

struct list {
    const struct object * object;
    const struct object * type;
    struct list_it * head;
    struct list_it * tail;
    unsigned int len;
};


struct list * list_create(const struct object * type);
void list_delete(struct list * list);
void list_delete_shallow(struct list * list);
struct list * list_copy(const struct list * list);

void list_append(struct list * list, void * object);
void list_prepend(struct list * list, void * object);

unsigned int list_len(const struct list * list);

/* Split the list at the given offset, and return tail part of the split. */
struct list * list_split(struct list * list, unsigned int offset);

void list_sort(struct list ** list);
void list_sort_custom(
    struct list ** list,
    int (*cmp)(const void *, const void *)
);

/* Make sure the list is sorted before calling dedup */
void list_dedup(struct list * list);

/*
    Make sure the list is sorted before calling dedup
    List list_dedup, but doesn't delete the underlying objects
*/
void list_dedup_shallow(struct list * list);

void list_join(struct list * list, struct list * rhs);

struct list * list_map(
    struct list * list,
    const struct object * new_type,
    void * (*f)(void *)
);

#endif