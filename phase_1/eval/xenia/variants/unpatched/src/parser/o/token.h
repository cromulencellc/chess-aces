#ifndef o_token_HEADER
#define o_token_HEADER
#include "object.h"
#include <stdint.h>
enum token_type {
  TOK_INVALID,
  TOK_ADD,
  TOK_REMOVE,
  TOK_SET_SUCCESSORS,
  TOK_SET_PREDECESSORS,
  TOK_GET,
  TOK_GET_PREDECESSORS,
  TOK_GET_SUCCESSORS,
  TOK_SET_PROPERTY,
  TOK_UPDATE_NODE_ID,
  TOK_PROPERTY_IS,
  TOK_HAS_PROPERTY,
  TOK_REMOVE_PROPERTY,
  TOK_SORT_BY,
  TOK_ADD_SUCCESSORS,
  TOK_ADD_PREDECESSORS,
  TOK_SQUARE_BRACKET_OPEN,
  TOK_SQUARE_BRACKET_CLOSE,
  TOK_PAREN_OPEN,
  TOK_PAREN_CLOSE,
  TOK_COMMA,
  TOK_TRANSITIVE_CLOSURE,
  TOK_DOT_GRAPH,
  TOK_FOR,
  TOK_IN,
  TOK_NOT,
  TOK_IDENTIFIER,
  TOK_END,
  TOK_WHERE,
  TOK_SEMICOLON,
  TOK_EXIT,
  TOK_STRING,
  TOK_NUMBER
};
extern const struct object *object_type_token;
struct token {
  const struct object *object;
  enum token_type type;
  char *text;
  unsigned int text_len;
  uint64_t num;
};
struct token *token_create(enum token_type type, const char *text,
                           uint32_t text_len);
void token_delete(struct token *);
struct token *token_copy(const struct token *token);
#endif