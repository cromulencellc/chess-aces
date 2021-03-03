#include "vertex.h"

#include <stdlib.h>
#include <string.h>

#include "container/ou64.h"
#include "rust.h"
#include "vertex_property.h"

const struct object vertex_o = {
    (object_delete_f) vertex_delete,
    (object_copy_f) object_not_copyable,
    (object_cmp_f) vertex_cmp
};

const struct object * object_type_vertex = &vertex_o;


struct vertex * vertex_create(uint64_t node_id) {
    struct vertex * vertex = (struct vertex *) unwrap_null(malloc(sizeof(struct vertex)));
    vertex->object = &vertex_o;
    vertex->node_id = node_id;
    vertex->properties = aa_tree_create(object_type_vertex_property);
    vertex->successors = (struct vertex **)
        unwrap_null(malloc(sizeof(struct vertex **) * DEFAULT_VERTEX_ARRAY_SIZE));
    vertex->predecessors = (struct vertex **)
        unwrap_null(malloc(sizeof(struct vertex **) * DEFAULT_VERTEX_ARRAY_SIZE));
    vertex->size_of_successors = DEFAULT_VERTEX_ARRAY_SIZE;
    vertex->size_of_predecessors = DEFAULT_VERTEX_ARRAY_SIZE;
    vertex->num_predecessors = 0;
    vertex->num_successors = 0;

    return vertex;
}

void vertex_delete(struct vertex * vertex) {
    object_delete(vertex->properties);
    free(vertex->successors);
    free(vertex->predecessors);
    free(vertex);
}

int vertex_cmp(const struct vertex * lhs, const struct vertex * rhs) {
    if (lhs->node_id < rhs->node_id) {
        return -1;
    }
    else if (lhs->node_id > rhs->node_id) {
        return 1;
    }
    else {
        return 0;
    }
}

struct list * vertex_get_successors_ref(struct vertex * vertex) {
    struct list * list = list_create(object_type_vertex);

    unsigned int i;
    for (i = 0; i < vertex->num_successors; i++) {
        list_append(list, vertex->successors[i]);
    }

    list_sort(&list);
    list_dedup_shallow(list);

    return list;
}

struct list * vertex_get_predecessors_ref(struct vertex * vertex) {
    struct list * list = list_create(object_type_vertex);

    unsigned int i;
    for (i = 0; i < vertex->num_predecessors; i++) {
        list_append(list, vertex->predecessors[i]);
    }

    list_sort(&list);
    list_dedup_shallow(list);

    return list;
}

void vertex_set_successors(struct vertex * vertex, struct list * successors) {
    if (successors->len > VERTEX_MAX_EDGES) {
        panic("Too many edges");
    }

    /* If we have any predecessors, make sure we remove ourself from their
       successor list */
    unsigned int i;
    for (i = 0; i < vertex->num_successors; i++) {
        vertex_remove_predecessor(vertex->successors[i], vertex->node_id);
    }

    if (vertex->size_of_successors < successors->len) {
        free(vertex->successors);
        vertex->successors =
            unwrap_null(malloc(sizeof(struct vertex **) * successors->len));
    }

    i = 0;
    struct list_it * it;
    for (it = successors->head; it != NULL; it = it->next) {
        struct vertex * successor = (struct vertex *) it->object;
        vertex->successors[i++] = successor;
        if (vertex_has_predecessor(successor, vertex) == 0) {
            vertex_push_predecessor(successor, vertex);
        }
    }
    vertex->num_successors = successors->len;
}

void vertex_set_predecessors(struct vertex * vertex, struct list * predecessors) {
    if (predecessors->len > VERTEX_MAX_EDGES) {
        panic("Too many edges");
    }

    /* If we have any predecessors, make sure we remove ourself from their
       successor list */
    unsigned int i;
    for (i = 0; i < vertex->num_predecessors; i++) {
        vertex_remove_successor(vertex->predecessors[i], vertex->node_id);
    }

    if (vertex->size_of_predecessors < predecessors->len) {
        free(vertex->predecessors);
        vertex->predecessors =
            unwrap_null(malloc(sizeof(struct vertex **) * predecessors->len));
    }

    i = 0;
    struct list_it * it;
    for (it = predecessors->head; it != NULL; it = it->next) {
        struct vertex * predecessor = (struct vertex *) it->object;
        vertex->predecessors[i++] = predecessor;
        if (vertex_has_successor(predecessor, vertex) == 0) {
            vertex_push_successor(predecessor, vertex);
        }
    }
    vertex->num_predecessors = predecessors->len;
}

int vertex_remove_successor(struct vertex * vertex, uint64_t node_id) {
    unsigned int i;
    for (i = 0; i < vertex->num_successors; i++) {
        if (vertex->successors[i]->node_id == node_id) {
            memmove(
                &vertex->successors[i],
                &vertex->successors[i + 1],
                sizeof(struct vertex **) * (vertex->num_successors - i - 1)
            );
            vertex->num_successors--;
            return 0;
        }
    }
    return 0;
}

int vertex_remove_predecessor(struct vertex * vertex, uint64_t node_id) {
    unsigned int i;
    for (i = 0; i < vertex->num_predecessors; i++) {
        if (vertex->predecessors[i]->node_id == node_id) {
            memmove(
                &vertex->predecessors[i],
                &vertex->predecessors[i + 1],
                sizeof(struct vertex **) * (vertex->num_predecessors - i - 1)
            );
            vertex->num_predecessors--;
            return 0;
        }
    }
    return 0;
}

int vertex_push_successor(struct vertex * vertex, struct vertex * successor) {
    if (vertex->size_of_successors == vertex->num_successors) {
        if (vertex->size_of_successors + 1 > VERTEX_MAX_EDGES) {
            return -1;
        }
        struct vertex ** new_successors =
            realloc(
                vertex->successors,
                sizeof(struct vertex **) * (vertex->size_of_successors + 1)
            );
        if (new_successors == NULL) {
            return -1;
        }
        vertex->successors = new_successors;
        vertex->size_of_successors += 1;
    }

#ifdef PATCH_NO_DUPLICATE_EDGES
    unsigned int i;
    for (i = 0; i < vertex->num_successors; i++) {
        if (vertex->successors[i]->node_id == successor->node_id) {
            panic("Tried to insert duplicate edges!");
        }
    }
#endif

    vertex->successors[vertex->num_successors++] = successor;
    return 0;
}

int vertex_push_predecessor(struct vertex * vertex, struct vertex * predecessor) {
    if (vertex->size_of_predecessors == vertex->num_predecessors) {
        if (vertex->size_of_predecessors + 1 > VERTEX_MAX_EDGES) {
            return -1;
        }
        struct vertex ** new_predecessors =
            realloc(
                vertex->predecessors,
                sizeof(struct vertex **) * (vertex->size_of_predecessors + 1)
            );
        if (new_predecessors == NULL) {
            return -1;
        }
        vertex->predecessors = new_predecessors;
        vertex->size_of_predecessors += 1;
    }

#ifdef PATCH_NO_DUPLICATE_EDGES
    unsigned int i;
    for (i = 0; i < vertex->num_predecessors; i++) {
        if (vertex->predecessors[i]->node_id == predecessor->node_id) {
            panic("Tried to insert duplicate edges!");
        }
    }
#endif

    vertex->predecessors[vertex->num_predecessors++] = predecessor;
    return 0;
}

int vertex_has_successor(struct vertex * vertex, struct vertex * successor) {
    unsigned int i;
    for (i = 0; i < vertex->num_successors; i++) {
        if (vertex->successors[i] == successor) {
            return 1;
        }
    }
    return 0;
}

int vertex_has_predecessor(struct vertex * vertex, struct vertex * predecessor) {
    unsigned int i;
    for (i = 0; i < vertex->num_predecessors; i++) {
        if (vertex->predecessors[i] == predecessor) {
            return 1;
        }
    }
    return 0;
}

int vertex_has_property(
    struct vertex * vertex,
    uint8_t * key,
    uint32_t key_length
) {
    uint8_t * value;
    uint32_t value_length;
    if (vertex_get_property(vertex, key, key_length, &value, &value_length) == 0)
        return 1;
    else
        return 0;
}

int vertex_get_property(
    struct vertex * vertex,
    uint8_t * key,
    uint32_t key_length,
    uint8_t ** value,
    uint32_t * value_length
) {
    uint8_t dummy[1];
    struct vertex_property * needle =
        vertex_property_create(key, key_length, dummy, 0);
    struct vertex_property * vp = aa_tree_fetch_ref(vertex->properties, needle);
    vertex_property_delete(needle);

    if (vp == NULL)
        return -1;
    else {
        *value = vp->value;
        *value_length = vp->value_length;
        return 0;
    }
}

int vertex_set_property(
    struct vertex * vertex,
    const uint8_t * key,
    uint32_t key_length,
    const uint8_t * value,
    uint32_t value_length
) {
    struct vertex_property * vp =
        vertex_property_create(key, key_length, value, value_length);
    /* delete it if it already exists */
    aa_tree_remove(vertex->properties, vp);
    return aa_tree_insert(vertex->properties, vp);
}

int vertex_remove_property(
    struct vertex * vertex,
    const uint8_t * key,
    uint32_t key_length
) {
    uint8_t dummy[1];
    struct vertex_property * needle =
        vertex_property_create(key, key_length, dummy, 0);
    aa_tree_remove(vertex->properties, needle);
    vertex_property_delete(needle);
    return 0;
}

struct aa_it * vertex_properties_iterator(struct vertex * vertex) {
    return aa_tree_iterator(vertex->properties);
}



const static struct object vertex_object_o = {
    (object_delete_f) vertex_object_delete,
    (object_copy_f) vertex_object_copy,
    object_not_comparable
};

const struct object * object_type_vertex_object = &vertex_object_o;

struct vertex_object * vertex_object_create(const struct vertex * vertex) {
    struct vertex_object * vo =
        (struct vertex_object *) malloc(sizeof(struct vertex_object));
    
    vo->object = &vertex_object_o;
    vo->node_id = vertex->node_id;
    vo->successors = list_create(object_type_ou64);
    vo->predecessors = list_create(object_type_ou64);

    unsigned int i;

    for(i = 0; i < vertex->num_successors; i++) {
        list_append(vo->successors, ou64_create(vertex->successors[i]->node_id));
    }

    for(i = 0; i < vertex->num_predecessors; i++) {
        list_append(vo->predecessors, ou64_create(vertex->predecessors[i]->node_id));
    }

    return vo;
}

void vertex_object_delete(struct vertex_object * vo) {
    list_delete(vo->successors);
    list_delete(vo->predecessors);
    free(vo);
}

struct vertex_object * vertex_object_copy(const struct vertex_object * vo) {
    struct vertex_object * nouvo =
        (struct vertex_object *) malloc(sizeof(struct vertex_object));
    
    nouvo->object = &vertex_object_o;
    nouvo->node_id = vo->node_id;
    nouvo->successors = list_copy(vo->successors);
    nouvo->predecessors = list_copy(vo->predecessors);

    return nouvo;
}

int vertex_object_cmp(
    const struct vertex_object * lhs,
    const struct vertex_object * rhs
) {
    if (lhs->node_id < rhs->node_id) {
        return -1;
    }
    else if (lhs->node_id > rhs->node_id) {
        return 1;
    }
    else {
        return 0;
    }
}

struct list * vertex_list_to_vertex_object_list(struct list * list) {
    return list_map(
        list,
        object_type_vertex_object,
        (void * (*)(void *)) vertex_object_create
    );
}

struct list * vertex_object_list_to_vertex_list(
    struct list * list,
    struct graph * graph
) {
    struct list * new_list = list_create(object_type_vertex);
    struct list_it * it;

    for (it = list->head; it != NULL; it = it->next) {
        struct vertex_object * vo = (struct vertex_object *) it->object;
        struct vertex * vertex = graph_get_vertex_ref(graph, vo->node_id);
        if (vo == NULL) {
            list_delete(new_list);
            return NULL;
        }
        list_append(new_list, vertex);
    }

    return new_list;
}