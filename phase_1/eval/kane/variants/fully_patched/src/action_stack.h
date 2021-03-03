#ifndef ACTION_STACK_H
#define ACTION_STACK_H
typedef struct Stack {
  int top;
  int cap;
  int *actions;
} Stack;
Stack *create_stack(int capacity);
void push(Stack *stack, int item);
void pop(Stack *stack);
void unload(Stack *stack, int item);
int peek(Stack *stack);
#endif
