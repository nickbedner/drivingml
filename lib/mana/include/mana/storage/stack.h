#pragma once

#include <stdlib.h>
#include <string.h>

#define STACK_INIT_CAPACITY 4

struct Stack {
  size_t top;
  size_t capacity;
  void** items;
};

global inline void stack_init(struct Stack* stack) {
  stack->top = 0;
  stack->capacity = STACK_INIT_CAPACITY;
  stack->items = (void**)malloc(sizeof(void*) * STACK_INIT_CAPACITY);
}

global inline void stack_delete(struct Stack* stack) {
  free(stack->items);
}

global inline int stack_empty(struct Stack* stack) {
  return stack->top == 0;
}

global inline size_t stack_size(struct Stack* stack) {
  return stack->top;
}

global inline size_t stack_capacity(struct Stack* stack) {
  return stack->capacity;
}

global inline void stack_resize(struct Stack* stack, size_t capacity) {
  void** newItems = (void**)realloc((char*)stack->items, sizeof(void*) * capacity);

  if (newItems) {
    stack->items = newItems;
    stack->capacity = capacity;
  }
}

global inline void stack_push(struct Stack* stack, void* item) {
  if (stack->capacity == stack->top)
    stack_resize(stack, stack->capacity * 2);

  stack->items[stack->top++] = item;
}

global inline void* stack_pop(struct Stack* stack) {
  if (stack_empty(stack))
    return NULL;

  if (stack->top > 0 && stack->top == stack->capacity / 4)
    stack_resize(stack, stack->capacity / 2);

  return stack->items[--stack->top];
}

// TODO: Check asm here could be optimized probably
global inline void* stack_peek(struct Stack* stack) {
  if (stack_empty(stack))
    return NULL;

  return stack->items[stack->top - 1];
}

global inline void stack_clear(struct Stack* stack) {
  stack->top = 0;
  stack->capacity = STACK_INIT_CAPACITY;
  stack_resize(stack, STACK_INIT_CAPACITY);
}
