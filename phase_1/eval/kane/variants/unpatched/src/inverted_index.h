#ifndef INVERTED_INDEX
#define INVERTED_INDEX
typedef struct Inv_Index {
  char **docs;
  char **words;
  int *flag;
} Inv_Index;
Inv_Index *new_iv(int len, int doc_count);
#endif
