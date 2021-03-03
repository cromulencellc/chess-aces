#include "vertex_property.h"

#include <stdlib.h>
#include <string.h>

#include "rust.h"

const struct object vertex_property_object = {
    (object_delete_f) vertex_property_delete,
    (object_copy_f) vertex_property_copy,
    (object_cmp_f) vertex_property_cmp
};

const struct object * object_type_vertex_property = &vertex_property_object;

struct vertex_property * vertex_property_create(
    const uint8_t * key,
    uint32_t key_length,
    const uint8_t * value,
    uint32_t value_length
) {
    struct vertex_property * vp =
        (struct vertex_property *) unwrap_null(
            malloc(sizeof(struct vertex_property)));
    
    vp->object = &vertex_property_object;
    vp->key_length = key_length;
    vp->value_length = value_length;
    vp->key = malloc(key_length);
    vp->value = malloc(value_length);
    memcpy(vp->key, key, key_length);
    memcpy(vp->value, value, value_length);

    return vp;
}

void vertex_property_delete(struct vertex_property * vp) {
    free(vp->key);
    free(vp->value);
    free(vp);
}

struct vertex_property * vertex_property_copy(const struct vertex_property * vp) {
    return vertex_property_create(vp->key, vp->key_length, vp->value, vp->value_length);
}

int vertex_property_cmp(
    const struct vertex_property * lhs,
    const struct vertex_property * rhs
) {
    if (lhs->key_length < rhs->key_length) {
        return -1;
    }
    else if (lhs->key_length > rhs->key_length) {
        return 1;
    }
    else {
        return memcmp(lhs->key, rhs->key, lhs->key_length);
    }
}