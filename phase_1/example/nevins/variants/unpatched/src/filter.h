#ifndef __FILTERS__
#define __FILTERS__
#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <strings.h>
#include "field.h"
#include "http.h"
#include "main.h"
#include "stream.h"
enum command {
  tr = 0,
  del,
  blk,
  cut,
  hh,
  tgt,
  src,
  bft,
  drp,
  mm_s,
  mm_t,
  mm_p
};
typedef struct mimetype {
  char *type;
  char *subtype;
  char *parameter;
  char *value;
} mimetype;
typedef struct filter {
  int location;
  enum command c;
  char *host;
  char *action;
  char *ft;
  char *element;
  char *from;
  char *to;
  char *cnt;
  char *mime_type;
  char *mime_subtype;
  char *mime_parameter;
  char *mime_value;
  int id;
  struct filter *next;
} filter;
char *list_filters(filter *root);
filter *parse_filter(char *filter_line);
int is_filter(char *uri);
int add_filter(filter **root, filter *f);
void free_filter(filter *f);
int apply_filters(HTTPRequest *req, filter *root, int is_request);
int filter_applies(field *field, char *content, filter *f, char *server,
                   char *uri, int is_request);
mimetype *parse_mimetype(char *mm);
#endif