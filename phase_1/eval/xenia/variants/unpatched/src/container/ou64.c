#include "ou64.h"
#include <stdlib.h>
#include "rust.h"
const static struct object ou64_object = {(object_delete_f)ou64_delete,
                                          (object_copy_f)ou64_copy,
                                          (object_cmp_f)ou64_cmp};
const struct object *object_type_ou64 = &ou64_object;
struct ou64 *ou64_create(uint64_t u64) {
  struct ou64 *ou64 = malloc(sizeof(struct ou64));
  ou64->object = &ou64_object;
  ou64->u64 = u64;
  return ou64;
}
void ou64_delete(struct ou64 *ou64) { free(ou64); }
struct ou64 *ou64_copy(const struct ou64 *ou64) {
  return ou64_create(ou64->u64);
}
int ou64_cmp(const struct ou64 *lhs, const struct ou64 *rhs) {
  if (lhs->u64 < rhs->u64) {
    return -1;
  } else if (lhs->u64 > rhs->u64) {
    return 1;
  }
  return 0;
}