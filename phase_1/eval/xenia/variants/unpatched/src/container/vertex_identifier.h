#ifndef vertex_identifier_HEADER
#define vertex_identifier_HEADER
#include <stdint.h>
#include "object.h"
extern const struct object *object_type_vertex_identifier;
enum vertex_identifier_type { VI_NODE_ID, VI_LABEL };
struct vertex_identifier {
  const struct object *object;
  enum vertex_identifier_type type;
  uint64_t node_id;
  char *label;
};
struct vertex_identifier *vertex_identifier_create_node_id(uint64_t node_id);
struct vertex_identifier *vertex_identifier_create_label(const char *label);
void vertex_identifier_delete(struct vertex_identifier *vi);
struct vertex_identifier *
vertex_identifier_copy(const struct vertex_identifier *);
int vertex_identifier_cmp(const struct vertex_identifier *lhs,
                          const struct vertex_identifier *rhs);
#endif