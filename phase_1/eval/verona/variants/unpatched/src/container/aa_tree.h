#ifndef aa_tree_HEADER
#define aa_tree_HEADER
typedef int (*aa_tree_cmp)(const void *, const void *);
typedef void (*aa_tree_del)(void *);
struct aa_node {
  int level;
  void *data;
  struct aa_node *left;
  struct aa_node *right;
};
struct aa_tree {
  struct aa_node *root;
  aa_tree_cmp cmp;
};
struct aa_tree *aa_tree_create(aa_tree_cmp cmp);
void aa_tree_delete(struct aa_tree *tree, aa_tree_del del);
int aa_tree_insert(struct aa_tree *tree, void *data);
void *aa_tree_fetch(struct aa_tree *tree, const void *data);
int aa_tree_remove(struct aa_tree *tree, int (*del)(void *), const void *data);
#endif