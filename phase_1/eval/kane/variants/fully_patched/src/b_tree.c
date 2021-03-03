#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "b_tree.h"
#include "directory.h"
#include "file.h"
#include "inverted_index.h"
#define SEARCH_PATH "/data/wiki/origins/"
B_Tree *create_node(char *word, char *doc_name, int dir_len);
B_Tree *insert(B_Tree *root, char *word, char *doc_name, int dir_len);
void get_dir_list(Directory *dir);
void inorder(B_Tree *root, Directory *dir);
void build_structs(Directory *dir);
void build_tree(Directory *dir, B_Tree *root);
B_Tree *search_for_word(B_Tree *root, char *word);
bool check_doc_list(B_Tree *root, char *doc_to_find, int dir_len);
void print_doc_words(Directory *dir);
void print_value(B_Tree *bt);
Search_Result *search_docs(B_Tree *root, char *query, Directory *dir);
Search_Result *start_search(char *query_string, char *path);
Search_Result *start_search(char *query_string, char *path) {
  B_Tree *rootptr = NULL;
  Search_Result *result_list = NULL;
  Directory *direct = malloc(sizeof(Directory));
  get_dir_list(direct);
  rootptr = insert(rootptr, "Howdy", "ROOT", direct->len);
  build_structs(direct);
  build_tree(direct, rootptr);
  result_list = search_docs(rootptr, query_string, direct);
  return result_list;
}
B_Tree *create_node(char *word, char *doc_name, int dir_len) {
  B_Tree *node = malloc(sizeof(B_Tree));
  node->word = malloc(strlen(word) + 1);
  node->value = malloc(sizeof(Value));
  node->value->doc = calloc(dir_len, sizeof(char *));
  int i;
  for (i = 0; i < dir_len; ++i) {
    if (i == 0) {
      node->value->doc[i] = malloc(strlen(doc_name) + 1);
      strncpy(node->value->doc[i], doc_name, strlen(doc_name));
      node->value->doc[i][strlen(doc_name)] = '\0';
    } else {
      node->value->doc[i] = malloc(strlen("") + 1);
      strncpy(node->value->doc[i], "", 1);
      node->value->doc[i][strlen("")] = '\0';
    }
  }
  node->value->occurences = calloc(dir_len, sizeof(int));
  for (i = 0; i < dir_len; ++i) {
    node->value->occurences[i] = 0;
  }
  strncpy(node->word, word, strlen(word));
  node->word[strlen(word)] = '\0';
  node->value->occurences[0] += 1;
  node->left = NULL;
  node->right = NULL;
  return node;
}
B_Tree *insert(B_Tree *root, char *word, char *doc_name, int dir_len) {
  if (root == NULL) {
    return create_node(word, doc_name, dir_len);
  }
  if (strlen(word) <= strlen(root->word)) {
    root->left = insert(root->left, word, doc_name, dir_len);
  } else if (strlen(word) > strlen(root->word)) {
    root->right = insert(root->right, word, doc_name, dir_len);
  }
  return root;
}
void inorder(B_Tree *root, Directory *dir) {
  if (root != NULL) {
    inorder(root->left, dir);
    int i;
    for (i = 0; i < dir->len; ++i) {
      printf("%s: occurs: root->value->occurences[%d]: %d time/s on this "
             "document root->value->doc[%d]: %s\n",
             root->word, i, root->value->occurences[i], i, root->value->doc[i]);
    }
    printf("\n");
    inorder(root->right, dir);
  }
}
void get_dir_list(Directory *direct) {
  DIR *d;
  direct->len = 0;
  struct dirent *dir;
  if ((d = opendir(SEARCH_PATH)) == NULL) {
    fprintf(stderr, "This dir does not exist\n");
    exit(1);
  }
  int i = 0, not_html_counter = -2;
  for (i = 0; (dir = readdir(d)) != NULL; ++i) {
    if (strcmp(dir->d_name, "..") != 0 && strcmp(dir->d_name, ".") != 0) {
      direct->len++;
    }
    if (strcmp(dir->d_name + (strlen(dir->d_name) - 4), ".txt") != 0) {
      not_html_counter++;
    }
  }
  closedir(d);
  if ((d = opendir(SEARCH_PATH)) == NULL) {
    fprintf(stderr, "Unable to reopen directory\n");
    exit(1);
  }
  int allocation_track = 0;
  direct->dir_list = calloc(direct->len - not_html_counter, sizeof(File));
  dir = readdir(d);
  for (i = 0; i < direct->len + 2 && dir != NULL; ++i, dir = readdir(d)) {
    if ((strcmp(dir->d_name, "..") == 0 || strcmp(dir->d_name, ".") == 0) ||
        strcmp(dir->d_name + (strlen(dir->d_name) - 4), ".txt") != 0) {
      continue;
    } else {
      if (allocation_track < (direct->len - not_html_counter)) {
        strncpy(direct->dir_list[allocation_track].name, dir->d_name, 256);
        printf("File[%d]: %s\n", allocation_track,
               direct->dir_list[allocation_track].name);
        allocation_track++;
      }
    }
  }
  direct->len -= not_html_counter;
  closedir(d);
}
void build_structs(Directory *dir) {
  int i, j;
  for (i = 0; i < dir->len; ++i) {
    int words = 0;
    FILE *fstream;
    char *dup_path =
        calloc(1, strlen(SEARCH_PATH) + strlen(dir->dir_list[i].name) + 5);
    snprintf(dup_path, strlen(SEARCH_PATH) + strlen(dir->dir_list[i].name) + 4,
             "%s%s", SEARCH_PATH, dir->dir_list[i].name);
    if ((fstream = fopen(dup_path, "r")) == NULL) {
      printf("There was a problem opening the file from the dir_list\n");
      exit(1);
    }
    fseek(fstream, 0L, SEEK_END);
    int size = ftell(fstream);
    rewind(fstream);
    dir->dir_list[i].contents = realloc(dir->dir_list[i].contents, size + 1);
    fread(dir->dir_list[i].contents, size, 1, fstream);
    dir->dir_list[i].contents[size] = '\0';
    rewind(fstream);
    char c;
    for (j = 0; j < size; ++j) {
      if ((c = fgetc(fstream)) == ' ' || c == '\n' || c == '\r') {
        words++;
      }
    }
    fclose(fstream);
    dir->dir_list[i].word_count = words;
    free(dup_path);
    dup_path = NULL;
  }
}
void build_tree(Directory *dir, B_Tree *root) {
  char *holder;
  B_Tree *place;
  bool debug = false;
  int i, j;
  for (i = 0; i < dir->len; ++i) {
    char *dup_cont = strdup(dir->dir_list[i].contents);
    holder = strtok(dup_cont, " ");
    for (j = 0; j < dir->dir_list[i].word_count; ++j) {
      if ((place = search_for_word(root, holder)) == NULL) {
        root = insert(root, holder, dir->dir_list[i].name, dir->len);
        if (debug) {
          printf("Diff Word, Different Dir or Diff Word, Same "
                 "Dir\n======================\nWord: %s\nDir: %s\n\n",
                 holder, dir->dir_list[i].name);
        }
      } else if (check_doc_list(place, dir->dir_list[i].name, dir->len)) {
        if (debug) {
          printf("Same Word, Same Dir\n======================\nWord: %s\nDir: "
                 "%s\n\n",
                 holder, dir->dir_list[i].name);
        }
        place->value->occurences[i] += 1;
        if (debug) {
          printf("place->value->doc[%d]: %s\n", i, place->value->doc[i]);
          printf("place->value->occurences[%d]: %d\n", i,
                 place->value->occurences[i]);
        }
      } else {
        if (debug) {
          printf("Same Word, Different Dir\n======================\nWord: "
                 "%s\nDir: %s\n\n",
                 holder, dir->dir_list[i].name);
        }
        place->value->occurences[i] += 1;
        place->value->doc[i] =
            realloc(place->value->doc[i], strlen(dir->dir_list[i].name) + 1);
        strncpy(place->value->doc[i], dir->dir_list[i].name,
                strlen(dir->dir_list[i].name));
        place->value->doc[i][strlen(dir->dir_list[i].name)] = '\0';
        if (debug) {
          printf("place->value->doc[%d]: %s\n", i, place->value->doc[i]);
          printf("place->value->occurences[%d]: %d\n", i,
                 place->value->occurences[i]);
        }
      }
      if ((holder = strtok(NULL, " ")) == NULL) {
        break;
      }
    }
  }
}
B_Tree *search_for_word(B_Tree *root, char *word) {
  if (root == NULL) {
    return NULL;
  } else if (strcasecmp(word, root->word) == 0) {
    return root;
  }
  if (strlen(root->word) < strlen(word)) {
    return search_for_word(root->right, word);
  }
  if (strlen(root->word) >= strlen(word)) {
    return search_for_word(root->left, word);
  }
  return root;
}
void print_doc_words(Directory *dir) {
  int i;
  for (i = 0; i < dir->len; ++i) {
    printf("%s\n", dir->dir_list[i].name);
    printf("%s\n", dir->dir_list[i].contents);
  }
}
bool check_doc_list(B_Tree *root, char *doc_to_find, int dir_len) {
  bool check = false;
  int i;
  for (i = 0; i < dir_len; ++i) {
    if (strcasecmp(root->value->doc[i], doc_to_find) == 0) {
      check = true;
      return check;
    }
  }
  return check;
}
void print_value(B_Tree *bt) {
  int i;
  printf("Word %s\n", bt->word);
  for (i = 0; bt->value->doc[i] != NULL; ++i) {
    printf("bt->value->doc[%d]: %s\n", i, bt->value->doc[i]);
  }
}
Search_Result *search_docs(B_Tree *root, char *query, Directory *dir) {
  char *holder;
  B_Tree *hold;
  Search_Result *sr = malloc(sizeof(Search_Result *));
  bool debug = false;
  sr->final_result = calloc(dir->len, sizeof(char *));
  ;
  sr->item_count = 0;
  int len = 1;
  printf("This is a query: %s\n", query);
  int i, j, k;
  for (i = 0; query[i] != '\0'; ++i) {
    if (query[i] == ' ') {
      len += 1;
    }
  }
  Inv_Index *iv = new_iv(len, dir->len);
  for (i = 0; i < dir->len; ++i) {
    iv->docs[i] = malloc(strlen(dir->dir_list[i].name) + 1);
    strncpy(iv->docs[i], dir->dir_list[i].name, strlen(dir->dir_list[i].name));
    iv->docs[i][strlen(dir->dir_list[i].name)] = '\0';
    printf("%s\n", iv->docs[i]);
  }
  printf("\n");
  holder = strtok(query, " ");
  for (i = 0; i < len; ++i) {
    if ((hold = search_for_word(root, holder)) == NULL) {
      printf("No search results\n");
      sr->final_result[sr->item_count] = malloc(strlen("No result found") + 1);
      strncpy(sr->final_result[sr->item_count], "No result found",
              strlen("No result found"));
      sr->final_result[sr->item_count][strlen("No result found")] = '\0';
      sr->item_count += 1;
      return sr;
    }
    iv->words[i] = malloc(strlen(hold->word) + 1);
    strncpy(iv->words[i], hold->word, strlen(hold->word));
    iv->words[i][strlen(hold->word)] = '\0';
    for (j = 0; j < dir->len; ++j) {
      if (debug)
        printf("Comparing %s to \" \"\n", hold->value->doc[j]);
      if (!strcmp(hold->value->doc[j], "")) {
        if (debug)
          printf("Continuing...\n");
        continue;
      }
      for (k = 0; k < dir->len; ++k) {
        if (debug)
          printf("Comparing %s to %s\n", hold->value->doc[j], iv->docs[k]);
        if (!strcmp(hold->value->doc[j], iv->docs[k])) {
          iv->flag[(i * dir->len) + k] = 1;
          if (debug)
            printf("iv->flag[%d] is now 1\nBreaking...\n", (i * dir->len) + k);
          break;
        }
      }
    }
    if ((holder = strtok(NULL, " ")) == NULL) {
      break;
    }
  }
  for (i = 0; i < dir->len; ++i) {
    int count = 0;
    for (j = 0; j < dir->len * len; j += dir->len) {
      if (iv->flag[j + i] != 1) {
        break;
      } else {
        count++;
      }
    }
    if (count == len) {
      sr->final_result[sr->item_count] = malloc(strlen(iv->docs[i]) + 1);
      strncpy(sr->final_result[sr->item_count], iv->docs[i],
              strlen(iv->docs[i]));
      sr->final_result[sr->item_count][strlen(iv->docs[i])] = '\0';
      printf("sr->final_result[%d]: %s\n", sr->item_count,
             sr->final_result[sr->item_count]);
      sr->item_count++;
    }
  }
  return sr;
}
