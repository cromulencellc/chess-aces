#ifndef B_TREE_H
#define B_TREE_H
#include "search_result.h"
#include "value.h"
typedef struct B_Tree {
  char *word;
  Value *value;
  int val_idx;
  struct B_Tree *left;
  struct B_Tree *right;
} B_Tree;
Search_Result *start_search(char *query_string, char *path);
#endif
