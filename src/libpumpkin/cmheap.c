#include "sys.h"
#include "heap.h"
#include "custom_malloc.h"
#include "debug.h"

struct heap_t {
  uint8_t *memory;
  uint8_t *start;
  uint8_t *end;
  uint32_t size;
  HEAP_INFO_t state;
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

  heap->state.pHeap = heap->start;
  heap->state.heapSize = size;
  CustomMallocInit(&heap->state);

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

void yyy(void *h) {
  heap_t *heap = (heap_t *)h;
  BD_t *iter = heap->state.avail_dll->next;
  if (iter->blkSize == 0) {
    debug(DEBUG_ERROR, "Heap", "available size became 0");
  }
}

void *heap_alloc(heap_t *heap, sys_size_t size) {
  BD_t *block;
  uint32_t realsize;
  uint8_t *p;

  p = CustomMalloc(&heap->state, size);
  if (p) {
    block = CustomBlock(&heap->state, p);
    realsize = block->blkSize;
    debug(DEBUG_TRACE, "Heap", "heap_alloc %u %u bytes %p to %p", realsize, (uint32_t)size, p, p + realsize - 1);
#if defined(HEAP_DEBUG)
    heap_debug_set(heap, p, realsize);
#endif
  }

  return p;
}

void *heap_realloc(heap_t *heap, void *p, sys_size_t size) {
  BD_t *block;
  uint32_t realsize;
  uint8_t *q;

  if (p) {
    block = CustomBlock(&heap->state, p);
    realsize = block->blkSize;
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
  BD_t *block;
  uint32_t realsize;
  uint8_t *p = pp;

  if (p) {
    block = CustomBlock(&heap->state, p);
    realsize = block->blkSize;
    debug(DEBUG_TRACE, "Heap", "heap_free %u bytes %p to %p", realsize, p, p + realsize - 1);
#if defined(HEAP_DEBUG)
    heap_debug_reset(heap, p, realsize);
#endif
    CustomFree(&heap->state, p);
  }
}

void heap_dump(heap_t *heap) {
  int fd;

  if ((fd = sys_create("heap.bin", SYS_WRITE, 0622)) != -1) {
    sys_write(fd, heap->start, heap->state.allocSize);
    sys_close(fd);
  }
}
