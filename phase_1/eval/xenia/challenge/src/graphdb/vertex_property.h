#ifndef vertex_property_HEADER
#define vertex_property_HEADER

#include "object.h"

#include <stdint.h>

extern const struct object * object_type_vertex_property;

struct vertex_property {
    const struct object * object;
    uint8_t * key;
    uint32_t key_length;
    uint8_t * value;
    uint32_t value_length;
};

struct vertex_property * vertex_property_create(
    const uint8_t * key,
    uint32_t key_length,
    const uint8_t * value,
    uint32_t value_length
);
void vertex_property_delete(struct vertex_property * vp);
struct vertex_property * vertex_property_copy(const struct vertex_property *);
int vertex_property_cmp(
    const struct vertex_property *,
    const struct vertex_property *
);

#endif