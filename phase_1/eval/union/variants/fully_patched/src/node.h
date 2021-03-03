#ifndef NODE_T
#define NODE_T
struct Node {
  void *data;
  struct Node *prev;
  struct Node *next;
};
typedef struct Node Node;
Node *create_node();
void destroy_node(Node *node);
#endif
