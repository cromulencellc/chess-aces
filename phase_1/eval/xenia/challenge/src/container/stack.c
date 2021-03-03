#include "stack.h"

#include <stdlib.h>

#include "rust.h"

#define INITIAL_STACK_SIZE 16
#define STACK_MAX_SIZE (1024 * 64)

const static struct object stack_object = {
    (object_delete_f) stack_delete,
    (object_copy_f) stack_copy,
    (object_cmp_f) object_not_comparable
};

const struct object * object_type_stack = &stack_object;


struct stack * stack_create() {
    struct stack * stack = (struct stack *) malloc(sizeof(struct stack));
    stack->object = &stack_object;
    stack->objects = malloc(sizeof(void *) * INITIAL_STACK_SIZE);
    stack->size = INITIAL_STACK_SIZE;
    stack->top = 0;
    return stack;
}


void stack_delete(struct stack * stack) {
    uint32_t i;
    for(i = 0; i < stack->top; i++) {
        object_delete(stack->objects[i]);
    }
    free(stack->objects);
    free(stack);
}


struct stack * stack_copy(const struct stack * stack) {
    struct stack * new_stack = stack_create();
    unsigned int i;
    for (i = 0; i < stack->top; i++) {
        stack_push(new_stack, object_copy(stack->objects[i]));
    }
    return new_stack;
}


int stack_push(struct stack * stack, void * item) {
    if (stack->top + 1 >= stack->size) {
        if (stack->size >= STACK_MAX_SIZE) {
            panic("Stack exhausted");
        }
        uint32_t new_size = stack->size * 2;
        void * new_items = unwrap_null(realloc(stack->objects, sizeof(void *) * new_size));
        stack->size = new_size;
        stack->objects = new_items;
    }
    stack->objects[stack->top++] = item;
    return 0;
}

void * stack_pop(struct stack * stack) {
    if (stack->top == 0) {
        void * result = stack->objects[0];
        stack->objects[0] = NULL;
        return result;
    }
    return stack->objects[--stack->top];
}

void * stack_peek_ref(struct stack * stack, uint32_t depth) {
    if ((depth >= stack->top) || (stack->top == 0)) {
        return NULL;
    }
    return (stack->objects[stack->top - depth - 1]);
}

uint32_t stack_num_items(const struct stack * stack) {
    return stack->top;
}