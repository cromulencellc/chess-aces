#ifndef o_parser_HEADER
#define o_parser_HEADER
#include "container/stack.h"
#include "object.h"
extern const struct object *object_type_parser;
struct parser {
  const struct object *object;
  struct stack *stack;
};
struct parser *parser_create();
void parser_delete(struct parser *parser);
struct parser *parser_copy(const struct parser *parser);
#endif