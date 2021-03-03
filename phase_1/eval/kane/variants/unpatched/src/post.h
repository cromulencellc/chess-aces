#ifndef POST_H
#define POST_H
#include "header.h"
#include "request.h"
typedef struct Post {
  int cont_len;
  char *cont_type;
} Post;
int get_cont_len(Header *headers);
void populate_req_body(Request *req, FILE *in);
#endif
