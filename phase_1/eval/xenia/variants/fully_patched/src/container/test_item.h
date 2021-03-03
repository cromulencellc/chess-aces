#ifndef test_item_HEADER
#define test_item_HEADER
#include <stdint.h>
#include "object.h"
extern const struct object *object_type_test_item;
struct test_item {
  const struct object *object;
  uint64_t num;
};
struct test_item *test_item_create(uint64_t num);
void test_item_delete(struct test_item *test_item);
struct test_item *test_item_copy(const struct test_item *test_item);
int test_item_cmp(const struct test_item *, const struct test_item *);
#endif