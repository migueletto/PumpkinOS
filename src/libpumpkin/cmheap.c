#include "sys.h"
#include "heap.h"
#include "custom_malloc.h"
#include "debug.h"

#define SMALL_BLOCK 64
#define HEAP_MARGIN 16*1024

struct heap_t {
  uint8_t *memory;
  uint8_t *start;
  uint8_t *end;
  uint32_t size;
  uint8_t *small_start;
  uint32_t small_size;
  uint32_t small_num_alloc;
  uint32_t *small_alloc;
  uint32_t small_alloc_size;
  HEAP_INFO_t state;
  int free;
#if defined(HEAP_DEBUG)
  uint32_t *bitfield;
#endif
};

heap_t *heap_init(uint8_t *memory, uint32_t size, uint32_t small_size, void *_wp) {
  heap_t *heap;
  uint32_t rem;
  uintptr_t p;

  debug(DEBUG_INFO, "Heap", "heap_init %u %u", size, small_size);

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

  p = (uintptr_t)heap->memory;
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

  if (small_size) {
    heap->small_start = heap->start;
    heap->small_size = small_size;
    heap->start += small_size;
    heap->size -= small_size;
    heap->small_num_alloc = small_size >> 11;
    heap->small_alloc = sys_calloc(heap->small_num_alloc, sizeof(uint32_t));
    heap->small_alloc_size = SMALL_BLOCK; // block 0 is pre-allocated and is not usable
  } else {
    heap->small_start = heap->start;
  }

  heap->state.pHeap = heap->start;
  heap->state.heapSize = heap->size;
  CustomMallocInit(&heap->state);

#if defined(HEAP_DEBUG)
  heap->bitfield = sys_calloc((size + 31) >> 5, sizeof(uint32_t));
#endif

  return heap;
}

void heap_finish(heap_t *heap) {
  if (heap) {
    debug(DEBUG_INFO, "Heap", "heap_finish");
    if (heap->memory && heap->free) sys_free(heap->memory);
    if (heap->small_alloc) sys_free(heap->small_alloc);
#if defined(HEAP_DEBUG)
    sys_free(heap->bitfield);
#endif
    sys_free(heap);
  }
}

void *heap_base(heap_t *heap) {
  return heap->small_start;
}

uint32_t heap_size(heap_t *heap) {
  return heap->small_size + heap->size;
}

#if defined(HEAP_DEBUG)
static void heap_debug_set(heap_t *heap, uint8_t *address, uint32_t size) {
  uint32_t i, index, offset;
  uint32_t mask;

  offset = address - heap->small_start;

  for (i = 0; i < size; offset++, i++) {
    index = offset >> 5;
    mask = 1 << (offset & 31);
    if (heap->bitfield[index] & mask) {
      debug(DEBUG_ERROR, "Heap", "set already mapped address %p", heap->small_start + offset);
      break;
    }
    heap->bitfield[index] |= mask;
  }
}

static void heap_debug_reset(heap_t *heap, uint8_t *address, uint32_t size) {
  uint32_t i, index, offset;
  uint32_t mask;

  offset = address - heap->small_start;

  for (i = 0; i < size; offset++, i++) {
    index = offset >> 5;
    mask = 1 << (offset & 31);
    if (!(heap->bitfield[index] & mask)) {
      debug(DEBUG_ERROR, "Heap", "reset unmapped address %p mask 0x%02X", heap->small_start + offset, mask);
      break;
    }
    heap->bitfield[index] &= mask ^ 0xffffffff;
  }
}

int heap_debug_access(void *p, uint32_t offset, uint32_t size, int read) {
  heap_t *heap = (heap_t *)p;
  uint32_t i, index, max;
  uint32_t mask;
  int valid = 1;

  max = heap->small_size + heap->size;
  for (i = 0; i < size; offset++, i++) {
    if (offset < max) {
      index = offset >> 5;
      mask = 1 << (offset & 31);
      if (!(heap->bitfield[index] & mask)) {
        debug(DEBUG_ERROR, "Heap", "%s access to unmapped address %p (0x%08X)", read ? "read" : "write", heap->small_start + offset, offset);
        valid = 0;
        break;
      }
    } else {
      debug(DEBUG_ERROR, "Heap", "%s access to large address %p (0x%08X)", read ? "read" : "write", heap->small_start + offset, offset);
      valid = 0;
      break;
    }
  }

  return valid;
}
#endif

void *heap_alloc(heap_t *heap, sys_size_t size) {
  uint32_t realsize, mask, i, j;
  uint8_t *p;

  if (heap->small_size && size <= SMALL_BLOCK) {
    realsize = SMALL_BLOCK;
    for (i = 0; i < heap->small_num_alloc; i++) {
      if (heap->small_alloc[i] != 0xffffffff) {
        if (i == 0) {
          // start the search at block 1, because block 0 is reserved as not usable
          j = 1;
          mask = 2;
        } else {
          j = 0;
          mask = 1;
        }
        for (; j < 32; j++) {
          if (!(heap->small_alloc[i] & mask)) {
            heap->small_alloc[i] |= mask;
            p = &heap->small_start[(i << 11) + (j << 6)];
            heap->small_alloc_size += realsize;
#if defined(HEAP_DEBUG)
            heap_debug_set(heap, p, realsize);
#endif
            debug(DEBUG_TRACE, "Heap", "heap_alloc %u %u bytes %p to %p (small %u/%u)", realsize, (uint32_t)size, p, p + realsize - 1, heap->small_alloc_size, heap->small_size);
            return p;
          }
          mask <<= 1;
        }
      }
    }
  } 

  if (heap->state.heapSize - heap->state.allocSize < HEAP_MARGIN) {
    heap_exhausted_error();
    return NULL;
  }

  p = CustomMalloc(&heap->state, size);
  if (p) {
    realsize = CustomBlockSize(&heap->state, p);
#if defined(HEAP_DEBUG)
    heap_debug_set(heap, p, realsize);
#endif
    debug(DEBUG_TRACE, "Heap", "heap_alloc %u %u bytes %p to %p (big %u/%u)", realsize, (uint32_t)size, p, p + realsize - 1, heap->state.allocSize, heap->state.heapSize);
  } else {
    heap_exhausted_error();
  }

  return p;
}

void *heap_realloc(heap_t *heap, void *pp, sys_size_t size) {
  uint32_t realsize;
  uint8_t *q, *p = pp;

  if (p) {
    if (heap->small_size && p >= heap->small_start && p < heap->start) {
      if (size <= SMALL_BLOCK) {
        debug(DEBUG_TRACE, "Heap", "heap_realloc keep %u bytes %p (small)", (uint32_t)size, p);
        return p;
      }
      q = heap_alloc(heap, size);
      if (q) {
        sys_memcpy(q, p, SMALL_BLOCK);
      }
      heap_free(heap, p);
      return q;
    }

    realsize = CustomBlockSize(&heap->state, p);
    if (size != realsize) {
      if ((q = heap_alloc(heap, size)) != NULL) {
        sys_memcpy(q, p, size < realsize ? size : realsize);
        heap_free(heap, p);
        p = q;
      } else {
        p = NULL;
      }
    }
  }

  return p;
}

void heap_free(heap_t *heap, void *pp) {
  uint32_t realsize, offset, mask, i;
  uint8_t *p = pp;

  if (p) {
    if (heap->small_size && p >= heap->small_start && p < heap->start) {
      offset = (uint32_t)(p - heap->small_start);
      i = offset >> 11;
      mask = 1 << ((offset & 0x7ff) >> 6);
      heap->small_alloc[i] &= mask ^ 0xffffffff;
      realsize = SMALL_BLOCK;
      heap->small_alloc_size -= realsize;
#if defined(HEAP_DEBUG)
      heap_debug_reset(heap, p, realsize);
#endif
      debug(DEBUG_TRACE, "Heap", "heap_free %u bytes %p to %p (small %u/%u)", realsize, p, p + realsize - 1, heap->small_alloc_size, heap->small_size);
      return;
    }

    realsize = CustomBlockSize(&heap->state, p);
#if defined(HEAP_DEBUG)
    heap_debug_reset(heap, p, realsize);
#endif
    CustomFree(&heap->state, p);
    debug(DEBUG_TRACE, "Heap", "heap_free %u bytes %p to %p (big %u/%u)", realsize, p, p + realsize - 1, heap->state.allocSize, heap->state.heapSize);
  }
}

void heap_dump(heap_t *heap) {
  int fd;

  if ((fd = sys_create("heap.bin", SYS_WRITE, 0644)) != -1) {
    sys_write(fd, heap->start, heap->state.allocSize);
    sys_close(fd);
  }
}
