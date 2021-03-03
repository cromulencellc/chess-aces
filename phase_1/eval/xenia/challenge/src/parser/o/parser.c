#include "parser.h"

#include <stdlib.h>

const static struct object parser_object = {
    (object_delete_f) parser_delete,
    (object_copy_f) parser_copy,
    (object_cmp_f) object_not_comparable
};

const struct object * object_type_parser = &parser_object;


struct parser * parser_create() {
    struct parser * parser = (struct parser *) malloc(sizeof(struct parser));
    parser->object = &parser_object;
    parser->stack = stack_create();
    return parser;
}


void parser_delete(struct parser * parser) {
    object_delete(parser->stack);
    free(parser);
}


struct parser * parser_copy(const struct parser * parser) {
    struct parser * new_parser = parser_create();
    object_delete(new_parser->stack);
    new_parser->stack = object_copy(parser->stack);
    return new_parser;
}
