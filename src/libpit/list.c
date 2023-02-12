#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include "plist.h"
#include "debug.h"
#include "xalloc.h"

node_t *list_new(void) {
  node_t *list;
  list = xcalloc(1, sizeof(node_t));
  return list;
}

node_t *list_add(node_t *list, void *element) {
  node_t *node = xcalloc(1, sizeof(node_t));
  if (node == NULL) {
    return NULL;
  }

  node->element = element;
  node->next = list->next;
  node->prev = list;

  if (node->next) {
    node->next->prev = node;
  }
  list->next = node;

  return node;
}

void list_append(node_t *list, void *element) {
  node_t *node, *prev;

  for (node = list_next(list), prev = NULL; node; prev = node, node = list_next(node));

  if (prev) {
    list_add(prev, element);
  } else {
    list_add(list, element);
  }
}

node_t *list_remove(node_t *list, node_t *node) {
  node_t *r = NULL;

  if (list && node) {
    if (node->next) {
      node->next->prev = node->prev;
    }
    if (node->prev) {
      node->prev->next = node->next;
    }
    r = node->next;
    xfree(node);
  }

  return r;
}

node_t *list_next(node_t *list) {
  return list->next;
}

void *list_element(node_t *list) {
  return list->element;
}

void **list_array(node_t *list) {
  node_t *node;
  void **array;
  int n;

  for (n = 0, node = list_next(list); node; node = list_next(node), n++);
  if (n == 0) return NULL;

  if ((array = xcalloc(n+1, sizeof(void *))) == NULL) {
    return NULL;
  }

  for (n = 0, node = list_next(list); node; node = list_next(node), n++) {
    array[n] = list_element(node);
  }

  return array;
}

int list_empty(node_t *list) {
  return list->next == NULL;
}
