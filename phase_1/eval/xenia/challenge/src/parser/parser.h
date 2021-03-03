#ifndef parser_HEADER
#define parser_HEADER

#include "container/list.h"
#include "container/stack.h"

#include "o/ast.h"
#include "o/parser.h"
#include "o/token.h"


struct parser * parser_create();
void parser_delete(struct parser * parser);


struct list * parse(struct list * tokens);

#endif