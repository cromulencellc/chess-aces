#ifndef __MATRIX_VARS__
#define __MATRIX_VARS__
#include <malloc.h>
#include "matrix.h"
typedef struct mvar {
  char *name;
  matrix *m;
  struct mvar *next;
  struct mvar *prev;
} mvar;
void add_var(char *name, matrix *A);
matrix *get_matrix_copy(char *name);
mvar *get_var(char *name);
#endif