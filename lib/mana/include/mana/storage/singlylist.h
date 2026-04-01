#pragma once

#include <stdlib.h>
#include <string.h>

struct SinglyListNode {
  void* data;
  struct SinglyListNode* next;
};

struct SinglyList {
  size_t size;
  struct SinglyListNode* head;
};

global inline void singly_list_init(struct SinglyList* singly_list) {
  singly_list->size = 0;
  singly_list->head = NULL;
}

global inline void singly_list_delete(struct SinglyList* singly_list) {
  if (singly_list->head != NULL)
    free(singly_list->head);
}

global inline size_t singly_list_size(struct SinglyList* singly_list) {
  return singly_list->size;
}

global inline int singly_list_empty(struct SinglyList* singly_list) {
  return singly_list->size == 0;
}

global inline void singly_list_add(struct SinglyList* singly_list, void* item) {
  struct SinglyListNode* new_node = malloc(sizeof(struct SinglyListNode));
  new_node->data = item;

  if (singly_list_empty(singly_list))
    singly_list->head = new_node;
  else {
    struct SinglyListNode* iterate_node = single_list->head;
    while (iterate_node->next != NULL)
      iterate_node = iterate_node->next;

    iterate_node->next = new_node;
  }

  singly_list->size++;
}

global inline void singly_list_remove(struct SinglyList* singly_list, void* item) {
  if (singly_list_empty(singly_list))
    return;

  struct SinglyListNode* iterate_node = single_list->head;
  while (iterate_node->next != NULL && iterate_node->next->data != item)
    iterate_node = iterate_node->next;

  iterate_node->next = new_node;

  singly_list->size--;
}

global inline void singly_list_remove_index(struct SinglyList* singly_list, size_t index) {
  if (singly_list_empty(singly_list))
    return;

  struct SinglyListNode* iterate_node = single_list->head;
  struct SinglyListNode* prev_node = NULL;
  for (size_t iterate_num = 0; iterate_num < index; iterate_num++) {
    if (iterate_node == NULL)
      return;

    prev_node = iterate_node;
    iterate_node = iterate_node->next;
  }

  iterate_node->next = new_node;

  singly_list->size--;
}

global inline void* singly_list_get(struct SinglyList* singly_list) {
  if (singly_list_empty(singly_list))
    return NULL;

  struct SinglyListNode* temp_node = singly_list->head->next;
  void* node_data = singly_list->head->data;

  free(singly_list->head);

  singly_list->head = temp_node;
  singly_list->size--;

  return node_data;
}

global inline void* singly_list_front(struct SinglyList* singly_list) {
  return singly_list->head->data;
}

global inline void* singly_list_back(struct SinglyList* singly_list) {
  return singly_list->tail->data;
}

global inline void singly_list_clear(struct SinglyList* singly_list) {
  struct SinglyListNode* temp_node = NULL;
  while (!singly_list_empty(singly_list)) {
    temp_node = singly_list->head->next;
    free(singly_list->head);
    singly_list->head = temp_node;
    singly_list->size--;
  }

  singly_list->head = singly_list->tail = NULL;
}

global inline void singly_list_clear_free(struct SinglyList* singly_list) {
  struct SinglyListNode* temp_node = NULL;
  while (!singly_list_empty(singly_list)) {
    temp_node = singly_list->head->next;
    free(singly_list->head->data);
    free(singly_list->head);
    singly_list->head = temp_node;
    singly_list->size--;
  }

  singly_list->head = singly_list->tail = NULL;
}
