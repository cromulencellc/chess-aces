#include "matrix_vars.h"
mvar *root = NULL;
void unlink_var(mvar *var) {
  if (!var) {
    return;
  }
  if (var->prev == NULL) {
    root = var->next;
    if (root) {
      root->prev = NULL;
    }
  } else {
    if (var->next != NULL) {
      var->next->prev = var->prev;
    }
    var->prev->next = var->next;
  }
  var->next = NULL;
  var->prev = NULL;
  return;
}
void add_var(char *name, matrix *A) {
  mvar *nv = NULL;
  if (!name || !A) {
    return;
  }
  nv = get_var(name);
  if (nv) {
    unlink_var(nv);
    free(nv->m->values);
    free(nv->m);
    free(nv->name);
    nv->m->values = NULL;
  } else {
    nv = calloc(1, sizeof(mvar));
    if (!nv) {
      return;
    }
  }
  nv->m = allocate_matrix(A->rows, A->columns);
  if (!nv->m) {
    free(nv);
    return;
  }
  for (int i = 0; i < A->rows * A->columns; i++) {
    nv->m->values[i] = A->values[i];
  }
  nv->name = strdup(name);
  if (!nv) {
    free_matrix(nv->m);
    free(nv);
    return;
  }
  nv->next = root;
  if (nv->next) {
    nv->next->prev = nv;
  }
  root = nv;
  return;
}
mvar *get_var(char *name) {
  mvar *walker = NULL;
  if (!name) {
    return NULL;
  }
  walker = root;
  while (walker) {
    if (strcmp(walker->name, name) == 0) {
      return walker;
    }
    walker = walker->next;
  }
  return walker;
}
matrix *get_matrix_copy(char *name) {
  mvar *ov = NULL;
  matrix *m = NULL;
  if (!name) {
    return NULL;
  }
  ov = get_var(name);
  if (!ov) {
    return NULL;
  }
  m = allocate_matrix(ov->m->rows, ov->m->columns);
  if (!m) {
    return NULL;
  }
  for (int i = 0; i < m->rows * m->columns; i++) {
    m->values[i] = ov->m->values[i];
  }
  return m;
}