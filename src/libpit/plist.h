#ifndef PIT_LIST_H
#define PIT_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct node {
  void *element;
  struct node *next;
  struct node *prev;
} node_t;

node_t *list_new(void);

node_t *list_add(node_t *list, void *element);

void list_append(node_t *list, void *element);

node_t *list_remove(node_t *list, node_t *node);

node_t *list_next(node_t *list);

void *list_element(node_t *list);

void **list_array(node_t *list);

int list_empty(node_t *list);

#ifdef __cplusplus
}
#endif

#endif
