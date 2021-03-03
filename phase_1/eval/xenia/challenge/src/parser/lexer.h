#ifndef lexer_HEADER
#define lexer_HEADER

#include "container/list.h"

#include <stdint.h>

extern const char * token_descriptions[];

/**
* Returns a list of <parser/o/token>
*/
int lexer(const char * text, struct list ** tokens);

#endif