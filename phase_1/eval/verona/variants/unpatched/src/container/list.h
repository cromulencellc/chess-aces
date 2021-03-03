#ifndef list_HEADER
#define list_HEADER
struct list_it {
  void *data;
  struct list_it *next;
  struct list_it *prev;
};
struct list {
  struct list_it *head;
  struct list_it *tail;
};
struct list *list_create();
void list_delete(struct list *list, void (*del)(void *));
void list_append(struct list *list, void *data);
void list_prepend(struct list *list, void *data);
#endif