#include "vertex_identifier.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "rust.h"

const static struct object vertex_identifier_object = {
    (object_delete_f) vertex_identifier_delete,
    (object_copy_f) vertex_identifier_copy,
    (object_cmp_f) vertex_identifier_cmp
};

const struct object * object_type_vertex_identifier;


struct vertex_identifier * vertex_identifier_create() {
    struct vertex_identifier * vertex_identifier =
        (struct vertex_identifier *) malloc(sizeof(struct vertex_identifier));

    vertex_identifier->object = &vertex_identifier_object;
    vertex_identifier->node_id = 0;
    vertex_identifier->label = NULL;
    return vertex_identifier;
}


struct vertex_identifier * vertex_identifier_create_node_id(uint64_t node_id) {
    struct vertex_identifier * vertex_identifier = vertex_identifier_create();
    vertex_identifier->type = VI_NODE_ID;
    vertex_identifier->node_id = node_id;
    return vertex_identifier;
}

struct vertex_identifier * vertex_identifier_create_label(const char * label) {
    struct vertex_identifier * vertex_identifier = vertex_identifier_create();
    vertex_identifier->type = VI_LABEL;
    vertex_identifier->label = unwrap_null(strdup(label));
    return vertex_identifier;
}

void vertex_identifier_delete(struct vertex_identifier * vi) {
    if (vi->type == VI_LABEL) {
        free(vi->label);
    }
    free(vi);
}

struct vertex_identifier * vertex_identifier_copy(
    const struct vertex_identifier * vi
) {
    if (vi->type == VI_NODE_ID) {
        return vertex_identifier_create_node_id(vi->node_id);
    }
    else {
        return vertex_identifier_create_label(vi->label);
    }
}

int vertex_identifier_cmp(
    const struct vertex_identifier * lhs,
    const struct vertex_identifier * rhs
) {
    if (lhs->type == VI_NODE_ID) {
        if (rhs->type == VI_LABEL) {
            return -1;
        }
        else if (lhs->node_id < rhs->node_id) {
            return -1;
        }
        else if (lhs->node_id > rhs->node_id) {
            return 1;
        }
        else {
            return 0;
        }
    }
    else {
        if (rhs->type == VI_NODE_ID) {
            return 1;
        }
        else {
            return strcmp(lhs->label, rhs->label);
        }
    }
}