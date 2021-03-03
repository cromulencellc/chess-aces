#include "aa_tree.h"
#include <stdlib.h>
#include "rust.h"
const struct object aa_tree_object = {(object_delete_f)aa_tree_delete,
                                      (object_copy_f)aa_tree_copy,
                                      (object_cmp_f)object_not_comparable};
const struct object *object_type_aa_tree = &aa_tree_object;
struct aa_tree *aa_tree_create(const struct object *type) {
  struct aa_tree *tree = malloc(sizeof(struct aa_tree));
  tree->object = &aa_tree_object;
  tree->type = type;
  tree->root = NULL;
  return tree;
}
void aa_tree_delete_node(struct aa_node *node) {
  if (node == NULL) {
    return;
  }
  if (node->left != NULL) {
    aa_tree_delete_node(node->left);
  }
  if (node->right != NULL) {
    aa_tree_delete_node(node->right);
  }
  object_delete(node->object);
  free(node);
}
void aa_tree_delete(struct aa_tree *tree) {
  aa_tree_delete_node(tree->root);
  free(tree);
}
struct aa_tree *aa_tree_copy(const struct aa_tree *at) {
  struct aa_tree *new_aa_tree = aa_tree_create(at->type);
  struct aa_it *it = aa_tree_iterator((struct aa_tree *)at);
  while (it != NULL) {
    aa_tree_insert(new_aa_tree, aa_it_object(it));
  }
  return new_aa_tree;
}
void *aa_tree_fetch_ref(struct aa_tree *tree, const void *object) {
  object_type_verify(object, tree->type);
  return aa_node_fetch_ref(tree->root, object);
}
int aa_tree_insert(struct aa_tree *tree, void *object) {
  object_type_verify(object, tree->type);
  struct aa_node *node = aa_node_insert(tree->root, object);
  if (node == NULL) {
    return -1;
  }
  tree->root = node;
  return 0;
}
int aa_tree_remove(struct aa_tree *tree, const void *object) {
  object_type_verify(object, tree->type);
  tree->root = aa_node_remove(tree->root, 0, object);
  return 0;
}
uint64_t aa_tree_num_items(const struct aa_tree *aa_tree) {
  if (aa_tree->root == NULL) {
    return 0;
  } else {
    return aa_node_num_items(aa_tree->root);
  }
}
const struct object aa_node_object = {(object_delete_f)aa_node_delete_,
                                      (object_copy_f)aa_node_copy,
                                      (object_cmp_f)object_not_comparable};
const struct object *object_type_aa_node;
struct aa_node *aa_node_create(void *object) {
  struct aa_node *aa_node = (struct aa_node *)malloc(sizeof(struct aa_node));
  aa_node->aa_node_object = &aa_node_object;
  aa_node->object = object;
  aa_node->level = 0;
  aa_node->left = NULL;
  aa_node->right = NULL;
  return aa_node;
}
void aa_node_delete_(struct aa_node *aa_node) {
  if (aa_node->object != NULL) {
    object_delete(aa_node->object);
  }
  free(aa_node);
}
struct aa_node *aa_node_copy(const struct aa_node *aa_node) {
  return aa_node_create(object_copy(aa_node->object));
}
struct aa_node *aa_node_skew(struct aa_node *node) {
  if ((node == NULL) || (node->left == NULL)) {
    return node;
  }
  if (node->left->level == node->level) {
    struct aa_node *L = node->left;
    node->left = L->right;
    L->right = node;
    return L;
  } else {
    return node;
  }
}
struct aa_node *aa_node_split(struct aa_node *node) {
  if ((node == NULL) || (node->right == NULL) || (node->right->right == NULL)) {
    return node;
  }
  if (node->level == node->right->right->level) {
    struct aa_node *R = node->right;
    node->right = R->left;
    R->left = node;
    R->level++;
    return R;
  } else {
    return node;
  }
}
struct aa_node *aa_node_insert(struct aa_node *node, void *object) {
  if (node == NULL) {
    return aa_node_create(object);
  }
  if (object_cmp(node->object, object) < 0) {
    node->left = aa_node_insert(node->left, object);
  } else if (object_cmp(node->object, object) > 0) {
    node->right = aa_node_insert(node->right, object);
  } else {
    return NULL;
  }
  node = aa_node_skew(node);
  node = aa_node_split(node);
  return node;
}
void *aa_node_fetch_ref(struct aa_node *node, const void *object) {
  if (node == NULL) {
    return NULL;
  }
  if (object_cmp(node->object, object) == 0) {
    return node->object;
  } else if (object_cmp(node->object, object) < 0) {
    return aa_node_fetch_ref(node->left, object);
  } else if (object_cmp(node->object, object) > 0) {
    return aa_node_fetch_ref(node->right, object);
  }
  return NULL;
}
struct aa_node *aa_node_decrease_level(struct aa_node *node) {
  if (node == NULL)
    return node;
  if ((node->left == NULL) || (node->right == NULL))
    return node;
  int should_be = node->left->level;
  if (node->right->level < should_be) {
    should_be = node->right->level;
  }
  should_be++;
  if (should_be < node->level) {
    node->level = should_be;
    if (should_be < node->right->level) {
      node->right->level = should_be;
    }
  }
  return node;
}
struct aa_node *aa_node_successor(struct aa_node *node) {
  if (node == NULL) {
    return NULL;
  }
  if (node->right == NULL) {
    return NULL;
  }
  node = node->right;
  while (node->left != NULL) {
    node = node->left;
  }
  return node;
}
struct aa_node *aa_node_predecessor(struct aa_node *node) {
  if (node == NULL) {
    return NULL;
  }
  if (node->left == NULL) {
    return NULL;
  }
  node = node->left;
  while (node->right != NULL) {
    node = node->right;
  }
  return node;
}
struct aa_node *aa_node_remove(struct aa_node *node, int already_deleted,
                               const void *object) {
  if (node == NULL) {
    return node;
  } else if (node->object == NULL) {
    object_delete(node);
    return NULL;
  } else if (object_cmp(node->object, object) < 0) {
    node->left = aa_node_remove(node->left, already_deleted, object);
  } else if (object_cmp(node->object, object) > 0) {
    node->right = aa_node_remove(node->right, already_deleted, object);
  } else {
    if ((node->left == NULL) && (node->right == NULL)) {
      object_delete(node);
      return NULL;
    } else if (node->left == NULL) {
      struct aa_node *L = aa_node_successor(node);
      void *tmp = node->object;
      node->object = L->object;
      L->object = tmp;
      node->right = aa_node_remove(node->right, 1, L->object);
    } else {
      struct aa_node *L = aa_node_predecessor(node);
      void *tmp = node->object;
      node->object = L->object;
      L->object = tmp;
      node->left = aa_node_remove(node->left, 1, L->object);
    }
  }
  node = aa_node_decrease_level(node);
  node = aa_node_skew(node);
  node->right = aa_node_skew(node->right);
  if (node->right != NULL) {
    node->right->right = aa_node_skew(node->right->right);
  }
  node = aa_node_split(node);
  node->right = aa_node_split(node->right);
  return node;
}
uint64_t aa_node_num_items(const struct aa_node *node) {
  if (node == NULL) {
    return 0;
  }
  uint64_t num = 1;
  if (node->left != NULL) {
    num += aa_node_num_items(node->left);
  }
  if (node->right != NULL) {
    num += aa_node_num_items(node->right);
  }
  return num;
}
const struct object aa_it_object_ = {(object_delete_f)aa_it_delete,
                                     (object_copy_f)aa_it_copy,
                                     (object_cmp_f)object_not_comparable};
struct aa_it *aa_it_create() {
  struct aa_it *it = (struct aa_it *)malloc(sizeof(struct aa_it));
  it->object = &aa_it_object_;
  it->parent = NULL;
  it->state = VISITED_SELF;
  it->node = NULL;
  return it;
}
void aa_it_delete(struct aa_it *it) { free(it); }
struct aa_it *aa_it_copy(const struct aa_it *it) {
  struct aa_it *new_it = aa_it_create();
  new_it->parent = it->parent;
  new_it->state = it->state;
  new_it->node = it->node;
  return new_it;
}
void aa_it_cleanup(struct aa_it *it) {
  if (it == NULL) {
    return;
  }
  aa_it_delete(it->parent);
  free(it);
}
struct aa_it *aa_tree_iterator(struct aa_tree *tree) {
  if (tree->root == NULL) {
    return NULL;
  }
  struct aa_it *it = aa_it_create();
  it->node = tree->root;
  return it;
}
struct aa_it *aa_it_next(struct aa_it *it) {
  if (it == NULL) {
    return NULL;
  }
  if ((it->state == VISITED_SELF) && (it->node->left != NULL)) {
    it->state = VISITED_LEFT;
    struct aa_it *new_it = aa_it_create();
    new_it->parent = it;
    new_it->node = it->node->left;
    return new_it;
  }
  if (((it->state == VISITED_SELF) || (it->state == VISITED_LEFT)) &&
      (it->node->right != NULL)) {
    it->state = VISITED_RIGHT;
    struct aa_it *new_it = aa_it_create();
    new_it->parent = it;
    new_it->node = it->node->right;
    return new_it;
  } else {
    struct aa_it *parent = it->parent;
    aa_it_delete(it);
    return aa_it_next(parent);
  }
}
void *aa_it_object(struct aa_it *it) {
  if (it->node != NULL) {
    return it->node->object;
  }
  return NULL;
}