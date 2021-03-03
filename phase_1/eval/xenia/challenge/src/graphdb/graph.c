#include "graph.h"

#include <stdio.h>
#include <stdlib.h>

#include "rust.h"
#include "vertex.h"

const struct object graph_object = {
    (object_delete_f) graph_delete,
    object_not_copyable,
    object_not_comparable
};

const struct object * object_type_graph = &graph_object;


struct graph * graph_create() {
    struct graph * graph = (struct graph *) malloc(sizeof(struct graph));
    if (graph == NULL) {
        unwrap_null(graph);
    }
    graph->object = &graph_object;
    graph->vertices = aa_tree_create(object_type_vertex);
    return graph;
}

void graph_delete(struct graph * graph) {
    aa_tree_delete(graph->vertices);
    free(graph);
}

int graph_add_vertex(struct graph * graph, uint64_t node_id) {
    struct vertex * vertex = graph_get_vertex_ref(graph, node_id);
    if (vertex == NULL) {
        vertex = vertex_create(node_id);
        aa_tree_insert(graph->vertices, vertex);
        return 0;
    }
    else {
        return -1;
    }
}

struct vertex * graph_get_vertex_ref(struct graph * graph, uint64_t node_id) {
    struct vertex * needle = vertex_create(node_id);
    struct vertex * result = aa_tree_fetch_ref(graph->vertices, needle);
    object_delete(needle);

    return result;
}

int graph_remove_vertex(struct graph * graph, uint64_t node_id) {
    struct vertex * vertex = graph_get_vertex_ref(graph, node_id);
    if (vertex == NULL) {
        panic("Attempted to remove non-existant vertex");
    }

    struct list * successors = vertex_get_successors_ref(vertex);
    struct list_it * it;
    for (it = successors->head; it != NULL; it = it->next) {
        struct vertex * v = (struct vertex *) it->object;
        vertex_remove_predecessor(v, node_id);
    }
    list_delete_shallow(successors);

    struct list * predecessors = vertex_get_predecessors_ref(vertex);
    for (it = predecessors->head; it != NULL; it = it->next) {
        struct vertex * v = (struct vertex *) it->object;
        vertex_remove_successor(v, node_id);
    }
    list_delete_shallow(predecessors);

    aa_tree_remove(graph->vertices, vertex);

    return 0;
}

int graph_transitive_closure(struct graph * graph) {
    int fixed_point = 0;

    while (!fixed_point) {
        fixed_point = 1;

        struct aa_it * it = aa_tree_iterator(graph->vertices);
        while (it != NULL) {
            struct vertex * vertex = (struct vertex *) aa_it_object(it);

            struct list * successors = vertex_get_successors_ref(vertex);

            struct list * successors_successors = list_create(object_type_vertex);
            struct list_it * sit;
            for (sit = successors->head; sit != NULL; sit = sit->next) {
                struct vertex * successor = (struct vertex *) sit->object;
                list_join(
                    successors_successors,
                    vertex_get_successors_ref(successor)
                );
            }

            list_delete_shallow(successors);

            int error = 0;
            list_sort(&successors_successors);
            list_dedup_shallow(successors_successors);
            for (sit = successors_successors->head; sit != NULL; sit = sit->next) {
                struct vertex * successor = (struct vertex *) sit->object;
                if (vertex_has_successor(vertex, successor) == 0) {
                    error = vertex_push_successor(vertex, successor);
                    if (error) {
                        break;
                    }
                }
            }

            list_delete_shallow(successors_successors);

            if (error) {
                aa_it_cleanup(it);
                break;
                return error;
            }

            it = aa_it_next(it);
        }
    }

    return 0;
}

int graph_dot_graph(struct graph * graph) {
    printf("BEGIN DOT_GRAPH\n");
    printf("digraph G {\n");
    struct aa_it * it = aa_tree_iterator(graph->vertices);
    while (it != NULL) {
        struct vertex * vertex = (struct vertex *) aa_it_object(it);

        printf("v%lx [label=\"0x%lx\"];\n", vertex->node_id, vertex->node_id);

        struct list * successors = vertex_get_successors_ref(vertex);
        struct list_it * sit;
        for (sit = successors->head; sit != NULL; sit = sit->next) {
            struct vertex * successor = (struct vertex *) sit->object;
            printf("v%lx -> v%lx;\n", vertex->node_id, successor->node_id);
        }
        list_delete_shallow(successors);

        it = aa_it_next(it);
    }
    printf("}\n");
    printf("END DOT_GRAPH\n");
    fflush(stdout);

    return 0;
}

struct aa_it * graph_iterator(struct graph * graph) {
    return aa_tree_iterator(graph->vertices);
}