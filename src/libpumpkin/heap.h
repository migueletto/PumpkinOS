#ifndef HEAP_H
#define HEAP_H

typedef struct heap_t heap_t;

heap_t *heap_init(uint8_t *base, uint32_t size, void *wp);
heap_t *heap_get(void);
void heap_finish(heap_t *heap);
void *heap_base(heap_t *heap);
uint32_t heap_size(heap_t *heap);
void heap_dump(heap_t *heap);
void heap_walk(heap_t *heap, void (*callback)(uint32_t *p, uint32_t size, uint32_t task), uint32_t task);
void *heap_alloc(heap_t *heap, sys_size_t size);
void *heap_realloc(heap_t *heap, void *p, sys_size_t size);
void heap_free(heap_t *heap, void *p);

// called from dlmalloc.c:
void *heap_morecore(void *h, sys_size_t size);
void heap_assert(void *h, const char *file, int line, const char *func, const char *cond);

void heap_exhausted_error(void);
void heap_assertion_error(char *msg);

#endif
