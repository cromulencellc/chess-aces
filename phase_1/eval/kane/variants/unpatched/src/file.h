#ifndef FILE_H
#define FILE_H
typedef struct File {
  char name[256];
  char *contents;
  int len;
  int word_count;
} File;
#endif
