#include "test_item.h"
#include <stdlib.h>
#include "rust.h"
const static struct object test_item_object = {
    (object_delete_f)test_item_delete, (object_copy_f)test_item_copy,
    (object_cmp_f)test_item_cmp};
const struct object *object_type_test_item = &test_item_object;
struct test_item *test_item_create(uint64_t num) {
  struct test_item *test_item = malloc(sizeof(struct test_item));
  test_item->object = &test_item_object;
  test_item->num = num;
  return test_item;
}
void test_item_delete(struct test_item *test_item) { free(test_item); }
struct test_item *test_item_copy(const struct test_item *test_item) {
  return test_item_create(test_item->num);
}
int test_item_cmp(const struct test_item *lhs, const struct test_item *rhs) {
  if (lhs->num < rhs->num) {
    return -1;
  } else if (lhs->num > rhs->num) {
    return 1;
  }
  return 0;
}