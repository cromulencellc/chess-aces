#ifndef graph_HEADER
#define graph_HEADER

#include <stdint.h>

#include "container/aa_tree.h"
#include "object.h"

extern const struct object * object_type_graph;

struct graph {
    const struct object * object;
    struct aa_tree * vertices;
};

struct graph * graph_create();
void graph_delete(struct graph *);


struct vertex * graph_get_vertex_ref(struct graph * graph, uint64_t node_id);
int graph_add_vertex(struct graph * graph, uint64_t node_id);
int graph_remove_vertex(struct graph * graph, uint64_t node_id);
int graph_transitive_closure(struct graph * graph);
int graph_dot_graph(struct graph * graph);

struct aa_it * graph_iterator(struct graph * graph);

#endif