#include "aa_tree.h"
#include <stdlib.h>
#include "error.h"
struct aa_tree *aa_tree_create(aa_tree_cmp cmp) {
  struct aa_tree *tree = malloc(sizeof(struct aa_tree));
  tree->root = NULL;
  tree->cmp = cmp;
  return tree;
}
void aa_tree_delete_node(struct aa_node *node, aa_tree_del delete) {
  if (node == NULL) {
    return;
  }
  if (node->left != NULL) {
    aa_tree_delete_node(node->left, delete);
  }
  if (node->right != NULL) {
    aa_tree_delete_node(node->right, delete);
  }
  delete (node->data);
  free(node);
}
void aa_tree_delete(struct aa_tree *tree, aa_tree_del delete) {
  aa_tree_delete_node(tree->root, delete);
  free(tree);
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
struct aa_node *aa_node_insert(struct aa_node *node,
                               int (*cmp)(const void *, const void *),
                               void *data) {
  if (node == NULL) {
    node = (struct aa_node *)malloc(sizeof(struct aa_node));
    node->level = 0;
    node->data = data;
    node->left = NULL;
    node->right = NULL;
    return node;
  }
  if (cmp(node->data, data) < 0) {
    node->left = aa_node_insert(node->left, cmp, data);
  } else if (cmp(node->data, data) > 0) {
    node->right = aa_node_insert(node->right, cmp, data);
  } else {
    return NULL;
  }
  node = aa_node_skew(node);
  node = aa_node_split(node);
  return node;
}
int aa_tree_insert(struct aa_tree *tree, void *data) {
  struct aa_node *node = aa_node_insert(tree->root, tree->cmp, data);
  if (node == NULL) {
    return ERROR_AA_TREE_DUPLICATE;
  }
  tree->root = node;
  return SUCCESS;
}
void *aa_node_fetch(struct aa_node *node,
                    int (*cmp)(const void *, const void *), const void *data) {
  if (node == NULL) {
    return NULL;
  }
  if (cmp(node->data, data) == 0) {
    return node->data;
  } else if (cmp(node->data, data) < 0) {
    return aa_node_fetch(node->left, cmp, data);
  } else if (cmp(node->data, data) > 0) {
    return aa_node_fetch(node->right, cmp, data);
  }
  return NULL;
}
void *aa_tree_fetch(struct aa_tree *tree, const void *data) {
  return aa_node_fetch(tree->root, tree->cmp, data);
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
struct aa_node *aa_node_delete(struct aa_node *node,
                               int (*cmp)(const void *, const void *),
                               int (*del)(void *), int already_deleted,
                               const void *data) {
  if (node == NULL) {
    return node;
  } else if (cmp(node->data, data) < 0) {
    node->left = aa_node_delete(node->left, cmp, del, already_deleted, data);
  } else if (cmp(node->data, data) > 0) {
    node->right = aa_node_delete(node->right, cmp, del, already_deleted, data);
  } else {
    if ((node->left == NULL) && (node->right == NULL)) {
      if (already_deleted == 0) {
        del(node->data);
      }
      free(node);
      return NULL;
    } else if (node->left == NULL) {
      struct aa_node *L = aa_node_successor(node);
      del(node->data);
      node->data = L->data;
      node->right = aa_node_delete(node->right, cmp, del, 1, L->data);
    } else {
      struct aa_node *L = aa_node_predecessor(node);
      del(node->data);
      node->data = L->data;
      node->left = aa_node_delete(node->left, cmp, del, 1, L->data);
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
int aa_tree_remove(struct aa_tree *tree, int (*del)(void *), const void *data) {
  tree->root = aa_node_delete(tree->root, tree->cmp, del, 0, data);
  return 0;
}