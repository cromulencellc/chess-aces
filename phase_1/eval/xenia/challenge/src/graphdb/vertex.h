#ifndef vertex_HEADER
#define vertex_HEADER

#include <stdint.h>

#include "container/list.h"
#include "graph.h"
#include "object.h"

#define DEFAULT_VERTEX_ARRAY_SIZE 4

#define VERTEX_MAX_EDGES 4096

extern const struct object * object_type_vertex;

struct vertex {
    const struct object * object;
    uint64_t node_id;
    struct aa_tree * properties;
    struct vertex ** successors;
    struct vertex ** predecessors;
    uint16_t size_of_successors;
    uint16_t num_successors;
    uint16_t size_of_predecessors;
    uint16_t num_predecessors;
};


struct vertex * vertex_create(uint64_t node_id);
void vertex_delete (struct vertex *);
struct vertex * vertex_copy(const struct vertex * vertex);
int vertex_cmp(const struct vertex * lhs, const struct vertex * rhs);

/* Returns a list of <graphdb/vertex> */
struct list * vertex_get_successors_ref(struct vertex * vertex);

/* Returns a list of <graphdb/vertex> */
struct list * vertex_get_predecessors_ref(struct vertex * vertex);

/* successors is a list of <graphdb/vertex> */
void vertex_set_successors(struct vertex * vertex, struct list * successors);

/* successors is a list of <graphdb/vertex> */
void vertex_set_predecessors(struct vertex * vertex, struct list * predecessors);

int vertex_remove_successor(struct vertex * vertex, uint64_t node_id);
int vertex_remove_predecessor(struct vertex * vertex, uint64_t node_id);

/** @return 0 on success, non-zero on failure */
int vertex_push_successor(struct vertex * vertex, struct vertex * successor);
/** @return 0 on success, non-zero on failure */
int vertex_push_predecessor(struct vertex * vertex, struct vertex * predecessor);

/** @return 1 if has successor, 0 otherwise */
int vertex_has_successor(struct vertex * vertex, struct vertex * successor);
/** @return 1 if has predecessor, 0 otherwise */
int vertex_has_predecessor(struct vertex * vertex, struct vertex * predecessor);

int vertex_has_property(
    struct vertex * vertex,
    uint8_t * key,
    uint32_t key_length
);
int vertex_get_property(
    struct vertex * vertex,
    uint8_t * key,
    uint32_t key_length,
    uint8_t ** value,
    uint32_t * value_length
);
int vertex_set_property(
    struct vertex * vertex,
    const uint8_t * key,
    uint32_t key_length,
    const uint8_t * value,
    uint32_t value_length
);
int vertex_remove_property(
    struct vertex * vertex,
    const uint8_t * key,
    uint32_t key_length
);
struct aa_it * vertex_properties_iterator(struct vertex * vertex);



extern const struct object * object_type_vertex_object;

/** A slower vertex with additional safety guarantees. This can be copied. */
struct vertex_object {
    const struct object * object;
    uint64_t node_id;
    /* A list of type <container/ou64> which contains vertex indices */
    struct list * successors;
    /* A list of type <container/ou64> which contains vertex indices */
    struct list * predecessors;
};

struct vertex_object * vertex_object_create(const struct vertex *);
void vertex_object_delete(struct vertex_object *);
struct vertex_object * vertex_object_copy(const struct vertex_object *);
int vertex_object_cmp(
    const struct vertex_object * lhs,
    const struct vertex_object * rhs
);

struct list * vertex_list_to_vertex_object_list(struct list * list);
struct list * vertex_object_list_to_vertex_list(
    struct list * list,
    struct graph * graph
);

#endif