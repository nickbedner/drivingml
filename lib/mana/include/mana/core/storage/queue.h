#pragma once

#include <stdlib.h>
#include <string.h>

struct Queue {
  size_t size;
  size_t capacity;
  size_t front;
  size_t back;
  void** items;
};

global inline void queue_init(struct Queue* queue, size_t capacity) {
  queue->capacity = capacity;
  queue->size = queue->front = 0;
  queue->back = capacity - 1;
  queue->items = (void**)malloc(sizeof(void*) * capacity);
}

global inline void queue_delete(struct Queue* queue) {
  free(queue->items);
}

global inline size_t queue_size(struct Queue* queue) {
  return queue->size;
}

global inline int queue_empty(struct Queue* queue) {
  return queue->size == 0;
}

global inline int queue_full(struct Queue* queue) {
  return queue->size == queue->capacity;
}

global inline void queue_push(struct Queue* queue, void* item) {
  if (queue->size == queue->capacity)
    return;

  queue->back = (queue->back + 1) % queue->capacity;
  queue->items[queue->back] = item;
  queue->size++;
}

global inline void* queue_pop(struct Queue* queue) {
  if (queue_empty(queue))
    return NULL;

  void* item_data = queue->items[queue->front];
  queue->front = (queue->front + 1) % queue->capacity;
  queue->size--;

  return item_data;
}

global inline void* queue_front(struct Queue* queue) {
  if (queue_empty(queue))
    return NULL;

  return queue->items[queue->front];
}

global inline void* queue_back(struct Queue* queue) {
  if (queue_empty(queue))
    return NULL;

  return queue->items[queue->back];
}

global inline void queue_clear(struct Queue* queue) {
  queue->size = queue->front = 0;
  queue->back = queue->capacity - 1;
}
