#ifndef stack_HEADER
#define stack_HEADER
#include <stdint.h>
struct stack {
  void **items;
  uint32_t size;
  uint32_t top;
};
struct stack *stack_create();
void stack_delete(struct stack *stack, void (*item_del)(void *));
int stack_push(struct stack *stack, void *);
void *stack_pop(struct stack *stack);
void *stack_peek(struct stack *stack, uint32_t depth);
#endif