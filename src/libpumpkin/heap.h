#ifndef HEAP_H
#define HEAP_H

typedef struct heap_t heap_t;

heap_t *heap_init(uint8_t *memory, uint32_t size, uint32_t small_size, void *wp);
heap_t *heap_get(void);
void heap_finish(heap_t *heap);
void *heap_base(heap_t *heap);
uint32_t heap_size(heap_t *heap);
void heap_dump(heap_t *heap);
void *heap_alloc(heap_t *heap, sys_size_t size);
void *heap_realloc(heap_t *heap, void *p, sys_size_t size);
void heap_free(heap_t *heap, void *p);

void heap_exhausted_error(void);
int heap_debug_access(void *p, uint32_t offset, uint32_t size, int read);

#endif
