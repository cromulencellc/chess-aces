#include "stack.h"
#include <stdlib.h>
#include "error.h"
#define INITIAL_STACK_SIZE 16
#define STACK_MAX_SIZE (1024 * 64)
struct stack *stack_create() {
  struct stack *stack = (struct stack *)malloc(sizeof(struct stack));
  stack->items = malloc(sizeof(void *) * INITIAL_STACK_SIZE);
  stack->size = INITIAL_STACK_SIZE;
  stack->top = 0;
  return stack;
}
void stack_delete(struct stack *stack, void (*item_del)(void *)) {
  uint32_t i;
  for (i = 0; i < stack->top; i++) {
    item_del(stack->items[i]);
  }
  free(stack->items);
  free(stack);
}
int stack_push(struct stack *stack, void *item) {
  if (stack->top + 1 >= stack->size) {
    if (stack->size >= STACK_MAX_SIZE) {
      return ERROR_STACK_EXHAUSTED;
    }
    uint32_t new_size = stack->size * 2;
    void *new_items = realloc(stack->items, sizeof(void *) * new_size);
    if (new_items == NULL) {
      return ERROR_OOM;
    }
    stack->size = new_size;
    stack->items = new_items;
  }
  stack->items[stack->top++] = item;
  return SUCCESS;
}
void *stack_pop(struct stack *stack) {
  if (stack->top == 0) {
    return NULL;
  }
  return stack->items[--stack->top];
}
void *stack_peek(struct stack *stack, uint32_t depth) {
  if ((depth >= stack->top) || (stack->top == 0)) {
    return NULL;
  }
  return (stack->items[stack->top - depth - 1]);
}