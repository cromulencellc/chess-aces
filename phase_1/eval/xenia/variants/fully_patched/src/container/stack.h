#ifndef stack_HEADER
#define stack_HEADER
#include <stdint.h>
#include "object.h"
extern const struct object *object_type_stack;
struct stack {
  const struct object *object;
  void **objects;
  uint32_t size;
  uint32_t top;
};
struct stack *stack_create();
void stack_delete(struct stack *stack);
struct stack *stack_copy(const struct stack *stack);
int stack_push(struct stack *stack, void *);
void *stack_pop(struct stack *stack);
void *stack_peek_ref(struct stack *stack, uint32_t depth);
uint32_t stack_num_items(const struct stack *stack);
#endif