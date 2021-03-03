#ifndef __FIELD__
#define __FIELD__
#include <ctype.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
typedef struct field {
  char *field_name;
  char *field_data;
  struct field *next;
} field;
field *parse_field(char *line);
void add_field(field **root, field *nf);
char *get_field_data(field **root, char *field_name);
#endif