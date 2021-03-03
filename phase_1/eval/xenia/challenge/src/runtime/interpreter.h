#ifndef interpreter_HEADER
#define interpreter_HEADER

#include "graphdb/graph.h"
#include "parser/o/ast.h"
#include "scope.h"

struct interpreter {
    struct graph * graph;
    struct scope * scope;
    int exited;
};

struct interpreter * interpreter_create();
void interpreter_delete(struct interpreter *);

int interpreter_add(struct interpreter * interpreter, const struct ast * ast);
int interpreter_remove(
    struct interpreter * interpreter,
    const struct ast * ast
);
struct list * interpreter_get(
    struct interpreter * interpreter,
    const struct ast * ast
);
struct list * ast_get_predecessors(
    struct interpreter * interpreter,
    struct ast * ast
);
struct list * ast_get_successors(
    struct interpreter * interpreter,
    struct ast * ast
);
struct list * interpreter_nodes(
    struct interpreter * interpreter,
    const struct ast * ast
);
int interpreter_set_successors(
    struct interpreter * interpreter,
    const struct ast * ast
);
int interpreter_set_predecessors(
    struct interpreter * interpreter,
    const struct ast * ast
);
struct list * interpreter_has_property(
    struct interpreter * interpreter,
    struct ast * ast
);
struct list * interpreter_property_is(
    struct interpreter * interpreter,
    struct ast * ast
);
int interpreter_set_property(
    struct interpreter * interpreter,
    struct ast * ast
);


void interpreter_print_vertex(const struct vertex * vertex);

struct list * interpreter_eval(struct interpreter * interpreter, struct ast * ast);

int interpreter_run(struct interpreter * interpreter, struct list * statements);

#endif