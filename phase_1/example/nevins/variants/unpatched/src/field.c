#include "field.h"
field *parse_field(char *line) {
  field *nf = NULL;
  char *end = NULL;
  if (line == NULL) {
    goto end;
  }
  nf = calloc(1, sizeof(field));
  if (!nf) {
    goto end;
  }
  end = strchr(line, ':');
  if (!end) {
    free(nf);
    nf = NULL;
    goto end;
  }
  nf->field_name = strndup(line, end - line);
  if (!nf->field_name) {
    free(nf);
    nf = NULL;
    goto end;
  }
  end++;
  while (isblank(*end)) {
    end++;
  }
  nf->field_data = strdup(end);
  if (!nf->field_data) {
    free(nf->field_name);
    free(nf);
    nf = NULL;
    goto end;
  }
  for (int i = 0; i < strlen(nf->field_name); i++) {
    if (!isascii(nf->field_name[i])) {
      free(nf->field_name);
      free(nf->field_data);
      free(nf);
      nf = NULL;
      goto end;
    }
  }
  for (int i = 0; i < strlen(nf->field_data); i++) {
    if (!isascii(nf->field_data[i])) {
      free(nf->field_name);
      free(nf->field_data);
      free(nf);
      nf = NULL;
      goto end;
    }
  }
end:
  return nf;
}
void add_field(field **root, field *nf) {
  field *t;
  if (!root || !nf) {
    goto end;
  }
  if (*root == NULL) {
    *root = nf;
    goto end;
  }
  t = *root;
  while (t->next) {
    t = t->next;
  }
  t->next = nf;
end:
  return;
}
char *get_field_data(field **root, char *field_name) {
  char *fn = NULL;
  field *walker = NULL;
  if (!root || !field_name) {
    goto end;
  }
  walker = *root;
  while (walker) {
    if (strcasecmp(walker->field_name, field_name) == 0) {
      fn = walker->field_data;
      goto end;
    }
    walker = walker->next;
  }
end:
  return fn;
}