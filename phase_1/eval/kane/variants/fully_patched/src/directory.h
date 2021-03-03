#ifndef DIRECTORY_H
#define DIRECTORY_H
#include "file.h"
typedef struct Directory {
  File *dir_list;
  int len;
} Directory;
void print_dict(Directory *dict);
#endif
