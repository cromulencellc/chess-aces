#include <stdio.h>
#include "directory.h"
void print_dict(Directory *dict) {
  int i;
  for (i = 0; i < dict->len; ++i) {
    printf("%s\n", dict->dir_list[i].name);
  }
}
