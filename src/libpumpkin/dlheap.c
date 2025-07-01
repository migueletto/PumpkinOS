#include "sys.h"
#include "heap.h"
#include "mutex.h"
#include "dlmalloc.h"
#include "debug.h"

#define HEAP_MARGIN 16*1024

struct heap_t {
  uint32_t size, pointer;
  uint8_t *memory;
  uint8_t *start;
  uint8_t *end;
  void *state;
  mutex_t *mutex;
  int free;
#if defined(HEAP_DEBUG)
  uint8_t *bitfield;
#endif
};

#if defined(HEAP_DEBUG)
extern void emupalmos_heap_debug(void *p);
#endif

heap_t *heap_init(uint8_t *memory, uint32_t size, void *_wp) {
  heap_t *heap;
  uint32_t rem;
  uint64_t p;

  debug(DEBUG_INFO, "Heap", "heap_init %u", size);

  if ((heap = sys_calloc(1, sizeof(heap_t))) == NULL) {
    return NULL;
  }

  heap->free = 1;
  if (memory) {
    heap->memory = memory;
    heap->free = 0;
  } else if ((heap->memory = sys_calloc(1, size + 16)) == NULL) {
    sys_free(heap);
    heap = NULL;
    return NULL;
  }

  p = (uint64_t)heap->memory;
  rem = p & 0xf;
  if (rem) {
    p += 16 - rem;
    heap->start = (uint8_t *)p;
    size -= 16 - rem;
  } else {
    heap->start = heap->memory;
  }

  heap->size = size;
  heap->end = heap->start + size;
  heap->state = sys_calloc(1, dlmalloc_state_size());

#if defined(HEAP_DEBUG)
  heap->bitfield = sys_calloc(1, size >> 3);
  emupalmos_heap_debug(heap);
#endif

  return heap;
}

void heap_finish(heap_t *heap) {
  if (heap) {
    debug(DEBUG_INFO, "Heap", "heap_finish");
    if (heap->memory && heap->free) sys_free(heap->memory);
    if (heap->state) sys_free(heap->state);
#if defined(HEAP_DEBUG)
    sys_free(heap->bitfield);
#endif
    sys_free(heap);
  }
}

void *heap_base(heap_t *heap) {
  return heap->start;
}

uint32_t heap_size(heap_t *heap) {
  return heap->size;
}

#if defined(HEAP_DEBUG)
extern void dlmalloc_info(void *h);

void heap_info(heap_t *heap) {
  dlmalloc_info(heap);
}

static void heap_debug_set(heap_t *heap, uint8_t *address, uint32_t size) {
  uint32_t i, index, offset;
  uint8_t mask;

  debug(DEBUG_TRACE, "Heap", "heap set %p %u", address, size);
  offset = address - heap->start;

  for (i = 0; i < size; offset++, i++) {
    index = offset >> 3;
    mask = 1 << (offset & 7);
    if (heap->bitfield[index] & mask) {
      debug(DEBUG_ERROR, "Heap", "set already mapped address %p", heap->start + offset);
    }
    heap->bitfield[index] |= mask;
  }
}

static void heap_debug_reset(heap_t *heap, uint8_t *address, uint32_t size) {
  uint32_t i, index, offset;
  uint8_t mask;

  debug(DEBUG_TRACE, "Heap", "heap reset %p %u", address, size);
  offset = address - heap->start;

  for (i = 0; i < size; offset++, i++) {
    index = offset >> 3;
    mask = 1 << (offset & 7);
    if (!(heap->bitfield[index] & mask)) {
      debug(DEBUG_ERROR, "Heap", "reset unmapped address %p mask 0x%02X", heap->start + offset, mask);
    }
    heap->bitfield[index] &= mask ^ 0xff;
  }
}

void heap_debug_access(void *p, uint32_t offset, uint32_t size, int write) {
  heap_t *heap = (heap_t *)p;
  uint32_t i, index;
  uint8_t mask;

  for (i = 0; i < size; offset++, i++) {
    index = offset >> 3;
    mask = 1 << (offset & 7);
    if (!(heap->bitfield[index] & mask)) {
      debug(DEBUG_ERROR, "Heap", "%s access to unmapped address %p (0x%08X)", write ? "write" : "read", heap->start + offset, offset);
    }
  }
}
#endif

void *heap_alloc(heap_t *heap, sys_size_t size) {
  sys_size_t realsize;
  sys_size_t *p, *q;

  p = dlmalloc(heap, size);
  if (p) {
    q = (sys_size_t *)p;
    realsize = (sys_size_t)(q[-1] & ~1);
    debug(DEBUG_TRACE, "Heap", "heap_alloc %u %u bytes %p to %p", (uint32_t)realsize, (uint32_t)size, p, (uint8_t *)p + realsize - 1);
#if defined(HEAP_DEBUG)
    //if ((size & 7) > 0) size += 8 - (size & 7);
    heap_debug_set(heap, (uint8_t *)p, (uint32_t)realsize);
#endif
  }
  return p;
}

void *heap_realloc(heap_t *heap, void *p, sys_size_t size) {
  sys_size_t realsize;
  sys_size_t *q;

  if (p) {
    q = (sys_size_t *)p;
    realsize = (sys_size_t)(q[-1] & ~1);
    //realsize -= 8;
    debug(DEBUG_TRACE, "Heap", "heap_free %u bytes %p to %p", (uint32_t)realsize, p, (uint8_t *)p + realsize - 1);
#if defined(HEAP_DEBUG)
    heap_debug_reset(heap, (uint8_t *)p, (uint32_t)realsize);
#endif

    p = dlrealloc(heap, p, size);
    debug(DEBUG_TRACE, "Heap", "heap_alloc %u %u bytes %p to %p", (uint32_t)realsize, (uint32_t)size, p, (uint8_t *)p + size - 1);
#if defined(HEAP_DEBUG)
    //if ((size & 7) > 0) size += 8 - (size & 7);
    heap_debug_set(heap, (uint8_t *)p, (uint32_t)size);
#endif
  }

  return p;
}

void heap_free(heap_t *heap, void *p) {
  sys_size_t realsize;
  sys_size_t *q;

  if (p) {
    q = (sys_size_t *)p;
    realsize = (sys_size_t)(q[-1] & ~1);
    //size -= 8;
    debug(DEBUG_TRACE, "Heap", "heap_free %u bytes %p to %p", (uint32_t)realsize, p, (uint8_t *)p + realsize - 1);
#if defined(HEAP_DEBUG)
    heap_debug_reset(heap, (uint8_t *)p, (uint32_t)realsize);
#endif
    dlfree(heap, p);
  }
}

void *dlmalloc_get_state(void *h) {
  heap_t *heap = (heap_t *)h;
  return heap->state;
}

void *heap_morecore(void *h, sys_size_t size) {
  heap_t *heap = (heap_t *)h;
  void *p = NULL;

  if ((heap->pointer + size) < heap->size - HEAP_MARGIN) {
    p = &heap->start[heap->pointer];
    heap->pointer += size;
    debug(DEBUG_TRACE, "Heap", "heap_morecore %u + %u < %u", heap->pointer, (uint32_t)size, heap->size - HEAP_MARGIN);
  } else {
    debug(DEBUG_ERROR, "Heap", "heap_morecore %u + %u >= %u", heap->pointer, (uint32_t)size, heap->size - HEAP_MARGIN);
    heap_exhausted_error();
  }

  return p;
}

void heap_dump(heap_t *heap) {
  int fd;

  if ((fd = sys_create("heap.bin", SYS_WRITE, 0622)) != -1) {
    sys_write(fd, heap->start, heap->pointer);
    sys_close(fd);
  }
}

typedef struct {
  sys_size_t prev_size;
  sys_size_t size;
} header_t;

void heap_walk(heap_t *heap, void (*callback)(uint32_t *p, uint32_t size, uint32_t task), uint32_t task) {
  uint32_t offset, current, size, used;
  header_t *header, *next;
  void *data;

  debug(DEBUG_TRACE, "Heap", "heap_walk task %u size %u %p", task, heap->pointer, heap->start);

  for (offset = 0; offset < heap->pointer;) {
    current = offset;
    header = (header_t *)&heap->start[current];
    size = (uint32_t)(header->size & ~1) - sizeof(header_t);
    data = &heap->start[current + sizeof(header_t)];
    offset += sizeof(header_t) + size;
    if (offset < heap->pointer) {
      next = (header_t *)&heap->start[offset];
      used = next->size & 1;
    } else {
      used = 0;
    }
    if (used) {
      callback(data, size, task);
    }
  }
  debug(DEBUG_TRACE, "Heap", "heap_walk end");
}

void heap_assert(void *h, const char *file, int line, const char *func, const char *cond) {
  char buf[256];

  // the heap is corrupted, reset it
  dlmalloc_init_state(h);

  if (func) {
    sys_snprintf(buf, sizeof(buf) - 1, "assert %s, %s, line %d: %s", file, func, line, cond);
  } else {
    sys_snprintf(buf, sizeof(buf) - 1, "assert %s, line %d: %s", file, line, cond);
  }
  debug(DEBUG_ERROR, "Heap", "%s", buf);

  heap_assertion_error(buf);
}

void heap_share(heap_t *heap, int share) {
  if (heap) {
    if (share) {
      heap->mutex = mutex_create("heap");
    } else {
      mutex_destroy(heap->mutex);
      heap->mutex = NULL;
    }
  }
}
