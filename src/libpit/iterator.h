#ifndef PIT_ITERATOR_H
#define PIT_ITERATOR_H

#define TAG_ITERATOR "iterator"

typedef struct iterator_t {
  char *tag;
  void *data;
  int (*position)(struct iterator_t *it, int index);
  int (*count)(struct iterator_t *it);
  int (*next)(struct iterator_t *it);
  int (*num_properties)(struct iterator_t *it);
  char *(*property_name)(struct iterator_t *it, int index);
  char *(*get_property_value)(struct iterator_t *it, int index);
  int (*set_property_value)(struct iterator_t *it, int index, char *value);
} iterator_t;

#endif
