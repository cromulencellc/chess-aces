#include "token.h"
#include <stdlib.h>
#include <string.h>
#include "rust.h"
const static struct object token_object = {(object_delete_f)token_delete,
                                           (object_copy_f)token_copy,
                                           (object_cmp_f)object_not_comparable};
const struct object *object_type_token = &token_object;
struct token *token_create(enum token_type type, const char *text,
                           uint32_t text_len) {
  struct token *token = (struct token *)malloc(sizeof(struct token));
  token->object = &token_object;
  token->type = type;
  token->text_len = text_len;
  token->text = malloc(text_len + 1);
  memcpy(token->text, text, text_len + 1);
  token->text[text_len] = 0;
  return token;
}
void token_delete(struct token *token) {
  free(token->text);
  free(token);
}
struct token *token_copy(const struct token *token) {
  struct token *t = (struct token *)malloc(sizeof(struct token));
  t->type = token->type;
  t->text_len = token->text_len;
  t->text = malloc(token->text_len + 1);
  memcpy(t->text, token->text, token->text_len + 1);
  t->text[token->text_len] = 0;
  t->num = token->num;
  return t;
}