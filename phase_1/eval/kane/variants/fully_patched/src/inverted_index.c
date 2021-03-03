#include <stdio.h>
#include <stdlib.h>
#include "inverted_index.h"
Inv_Index *new_iv(int len, int doc_count) {
  Inv_Index *iv = malloc(sizeof(Inv_Index));
  iv->words = calloc(len, sizeof(char *));
  iv->docs = calloc(doc_count, sizeof(char *));
  iv->flag = calloc(doc_count * len, sizeof(int));
  return iv;
}
