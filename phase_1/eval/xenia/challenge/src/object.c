#include "object.h"

#include <stdlib.h>
#include <stdio.h>

#include "rust.h"

#include "container/list.h"
#include "container/ou64.h"
#include "graphdb/vertex.h"
#include "graphdb/vertex_property.h"
#include "runtime/scope.h"
#include "parser/o/ast.h"
#include "parser/o/parser.h"


void * object_not_copyable(const void * object) {
    panic("Tried to copy a non-copyable object. Exiting safely.\n");
    return NULL;
}

int object_not_comparable(const void * lhs, const void * rhs) {
    panic("Tried to compare a non-comparable object. Exiting safely.\n");
    return 0;
}

int object_is(const void * object, const struct object * object_type) {
    if (*((const struct object **) object) == object_type) {
        return 1;
    }
    else {
        return 0;
    }
}

const char * object_type_debug(const struct object * type) {
    if (type == object_type_aa_tree) return "aa_tree";
    else if (type == object_type_aa_node) return "aa_node";
    else if (type == object_type_ast) return "ast";
    else if (type == object_type_graph) return "graph";
    else if (type == object_type_list) return "list";
    else if (type == object_type_ou64) return "ou64";
    else if (type == object_type_parser) return "parser";
    else if (type == object_type_parser) return "scope";
    else if (type == object_type_stack) return "stack";
    else if (type == object_type_token) return "token";
    else if (type == object_type_variable) return "variable";
    else if (type == object_type_vertex) return "vertex";
    else if (type == object_type_vertex_identifier) return "vertex_identifier";
    else if (type == object_type_vertex_property) return "vertex_property";
    else if (type == object_type_vertex_object) return "vertex_object";
    return "unknown";
}

const char * object_debug(const void * object) {
    return object_type_debug(*((const struct object **) object));
}

void object_type_verify_(
    const void * object,
    const struct object * object_type,
    const char * file,
    const char * function,
    unsigned int line
) {
    if (!object_is(object, object_type)) {
        char buf[1024];
        snprintf(
            buf,
            1024,
            "[OBJECT TYPE MISMATCH] %s:%s:%u (%s %p) (%s %p)\n",
            file,
            function,
            line,
            object_type_debug((*(const struct object **) object)),
            (*(const struct object **) object),
            object_type_debug(object_type),
            object_type
        );
        panic(buf);
    }
}

void object_delete(void * obj) {
    (*((struct object **) obj))->delete(obj);
}


void * object_copy(const void * obj) {
    return (*((struct object **) obj))->copy(obj);
}


int object_cmp(const void * lhs, const void * rhs) {
    return (*((struct object **) rhs))->cmp(lhs, rhs);
}