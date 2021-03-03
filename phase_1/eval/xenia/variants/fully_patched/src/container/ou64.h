#ifndef ou64_HEADER
#define ou64_HEADER
#include <stdint.h>
#include "object.h"
extern const struct object *object_type_ou64;
struct ou64 {
  const struct object *object;
  uint64_t u64;
};
struct ou64 *ou64_create(uint64_t u64);
void ou64_delete(struct ou64 *ou64);
struct ou64 *ou64_copy(const struct ou64 *ou64);
int ou64_cmp(const struct ou64 *, const struct ou64 *);
#endif