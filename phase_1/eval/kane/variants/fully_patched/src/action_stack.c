#include <stdio.h>
#include <stdlib.h>
#include "action_stack.h"
Stack *create_stack(int capacity) {
  Stack *stack = malloc(sizeof(Stack));
  stack->cap = capacity;
  stack->top = -1;
  stack->actions = calloc(1, sizeof(int) * capacity);
  return stack;
}
void push(Stack *stack, int item) {
  if (stack->cap <= stack->top) {
    fprintf(stderr, "The stack is at capacity\n");
    return;
  } else {
    stack->actions[++stack->top] -= item;
  }
}
void pop(Stack *stack) {
  if (stack->top < 0) {
    fprintf(stderr, "There is nothing to pop off\n");
    return;
  } else {
    stack->actions[stack->top] = 0;
    stack->top--;
  }
}
void unload(Stack *stack, int item) {
  while (stack->top > -1) {
    pop(stack);
    stack->top--;
  }
  printf("All items popped off the stack\n");
}
int peek(Stack *stack) {
  printf("The item at the top of the stack is %d\n", stack->top);
  return stack->top;
}
void destroy_stack() {}
