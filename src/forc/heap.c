#include "sys.h"
#include "heap.h"

typedef struct {
  uint32_t prev;
  uint32_t use;
  uint32_t size;
  uint8_t data[];
} block_t;

struct heap_t {
  uint8_t *start;
  uint32_t size;
};

heap_t *heap_init(uint8_t *start, uint32_t size) {
  heap_t *heap;
  block_t *b;

  if ((heap = sys_calloc(1, sizeof(heap_t))) != NULL) {
    heap->start = start;
    heap->size = size;
    b = (block_t *)start;
    b->prev = 0xffffffff;
    b->use = 0;
    b->size = size - sizeof(block_t);
  }

  return heap;
}

void heap_finish(heap_t *heap) {
  if (heap) sys_free(heap);
}

/*
void heap_dump(heap_t *heap) {
  uint8_t *p, *end;
  block_t *b;

  end = heap->start + heap->size;
  for (p = heap->start; p < end;) {
    b = (block_t *)p;
    if (b->prev == 0xffffffff) {
      fprintf(stderr, "block %8u first         use %u size %4u\n", (uint32_t)(p - heap->start), b->use, b->size);
    } else {
      fprintf(stderr, "block %8u prev %8u use %u size %4u\n", (uint32_t)(p - heap->start), b->prev, b->use, b->size);
    }
    p += sizeof(block_t) + b->size;
  }
}
*/

uint8_t *heap_alloc(heap_t *heap, uint32_t size) {
  uint8_t *end, *p, *prev;
  //uint32_t orig;
  uint32_t remainder;
  block_t *b, *next;

  // heap: 1024 bytes
  // [ffff][0][1012][block]
  // |--------1024--------|
  // alloc 8
  // [ffff][1][8][block] [0][0][992][block]
  // |--------20-------| |------1004------|

  if (heap) {
    //orig = size;
    remainder = size % 4;
    if (remainder) size += 4 - remainder;
    end = heap->start + heap->size;
    prev = NULL;
    for (p = heap->start; p < end;) {
      b = (block_t *)p;
      if (b->use || size > b->size) {
        prev = p;
        p += sizeof(block_t) + b->size;
        continue;
      }
      remainder = b->size - size;
      if (remainder <= sizeof(block_t)) {
        size += remainder;
        b->prev = prev ? prev - heap->start : 0xffffffff;
        b->use = 1;
        b->size = size;
        sys_memset(b->data, 0, b->size);
        //fprintf(stderr, "alloc %u (%u) %p\n", size, orig, p + sizeof(block_t));
        return p + sizeof(block_t);
      }
      next = (block_t *)(p + sizeof(block_t) + size);
      next->prev = (uint8_t *)b - heap->start;
      next->use = 0;
      next->size = b->size - (sizeof(block_t) + size);
      b->prev = prev ? prev - heap->start : 0xffffffff;
      b->use = 1;
      b->size = size;
      sys_memset(b->data, 0, b->size);
      //fprintf(stderr, "alloc %u (%u) %p\n", size, orig, p + sizeof(block_t));
      return p + sizeof(block_t);
    }
  }

  //fprintf(stderr, "no space left on heap\n");
  return NULL;
}

int heap_free(heap_t *heap, uint8_t *p) {
  uint32_t *q;
  uint8_t *end;
  block_t *b, *prev, *next;

  if (heap) {
    //fprintf(stderr, "free %p\n", p);
    end = heap->start + heap->size;
    if (p < heap->start + sizeof(block_t) || p >= end) {
      //fprintf(stderr, "free invalid address %p\n", p);
      return -1;
    }

    q = (uint32_t *)p;
    b = (block_t *)&q[-3];
    if (!b->use) {
      //fprintf(stderr, "free unused address %u\n", (uint32_t)(p - heap->start));
      return -1;
    }

    if ((uint8_t *)b == heap->start) {
      // free first block
      next = (block_t *)(p + b->size);
      if ((uint8_t *)next < end) {
        if (next->use) {
          // next block is used: | [used] [used] ... | -> | [free] [used] ... |
          b->use = 0;
          sys_memset(b->data, 0, b->size);
          //fprintf(stderr, "free first block, next block is used\n");
        } else {
          // | next block is free: [used] [free] ... | -> | [free + free] ... |
          // merge with next block
          b->use = 0;
          b->size += sizeof(block_t) + next->size;
          sys_memset(b->data, 0, b->size);
          //fprintf(stderr, "free first block, merge with next block\n");
        }
      } else {
        // free only block: | [used] | -> | [free] |
        b->use = 0;
        sys_memset(b->data, 0, b->size);
        //fprintf(stderr, "free only block");
      }
      return 0;
    }

    if (p + b->size == end) {
      // free last block
      prev = (block_t *)(heap->start + b->prev);
      if (prev->use) {
        // prev block is used: | ... [used] [used] | -> | ... [used] [free] |
        b->use = 0;
        sys_memset(b->data, 0, b->size);
        //fprintf(stderr, "free last block, prev block is used\n");
      } else {
        // prev block is free: | ... [free] [used] | -> | ... [free + free] |
        // merge with prev block
        prev->size += sizeof(block_t) + b->size;
        sys_memset(prev->data, 0, prev->size);
        //fprintf(stderr, "free last block, prev block is free\n");
      }
      return 0;
    }

    // free intermediate block
    prev = (block_t *)(heap->start + b->prev);
    next = (block_t *)(p + b->size);
    if (prev->use && next->use) {
      // prev and next blocks are used: | ... [used] [used] [used] ... | -> | ... [used] [free] [used] ... |
      b->use = 0;
      sys_memset(b->data, 0, b->size);
      //fprintf(stderr, "free intermediate block, prev and next blocks are used\n");
      return 0;
    }
    if (prev->use && !next->use) {
      // prev used, next free: | ... [used] [used] [free] ... | -> | ... [used] [free + free] ... |
      // merge with next block
      b->use = 0;
      b->size += sizeof(block_t) + next->size;
      sys_memset(b->data, 0, b->size);
      //fprintf(stderr, "free intermediate block, merge with next block\n");
      return 0;
    }
    if (!prev->use && next->use) {
      // prev free, next used: | ... [free] [used] [used] ... | -> | ... [free + free] [used] ... |
      // merge with prev block
      prev->size += sizeof(block_t) + b->size;
      sys_memset(prev->data, 0, prev->size);
      //fprintf(stderr, "free intermediate block, merge with prev block\n");
      return 0;
    }
    // prev and next blocks are free: | ... [free] [used] [free] ... | -> | ... [free + free + free] ... |
    // merge the three blocks
    prev->size += sizeof(block_t) + b->size + sizeof(block_t) + next->size;
    sys_memset(prev->data, 0, prev->size);
    //fprintf(stderr, "free intermediate block, merge the three blocks\n");
    return 0;
  }

  return -1;
}

int heap_inc(heap_t *heap, uint8_t *p) {
  uint32_t *q;
  uint8_t *end;
  block_t *b;

  if (heap) {
    end = heap->start + heap->size;
    if (p < heap->start + sizeof(block_t) || p >= end) {
      //fprintf(stderr, "inc invalid address %p\n", p);
      return -1;
    }

    q = (uint32_t *)p;
    b = (block_t *)&q[-3];
    //fprintf(stderr, "inc %p %u -> %d\n", p, b->use, b->use+1);
    b->use++;
    return 0;
  }

  return -1;
}

int heap_dec(heap_t *heap, uint8_t *p) {
  uint32_t *q;
  uint8_t *end;
  block_t *b;

  if (heap) {
    end = heap->start + heap->size;
    if (p < heap->start + sizeof(block_t) || p >= end) {
      //fprintf(stderr, "dec invalid address %p\n", p);
      return -1;
    }

    q = (uint32_t *)p;
    b = (block_t *)&q[-3];
    if (!b->use) {
      //fprintf(stderr, "dec unused address %p\n", p);
      return -1;
    }
    //fprintf(stderr, "dec %p %u -> %d\n", p, b->use, b->use-1);
    if (b->use == 1) {
      // if it is the last reference, free the block
      heap_free(heap, p);
    } else {
      // otherwise, just decrement the count
      b->use--;
    }
    return 0;
  }

  return -1;
}

/*
int main(void) {
  heap_t *heap;
  uint8_t *buf;

  buf = calloc(1, 1024);
  heap = heap_init(buf, 1024);
  heap_dump(heap);
  uint8_t *p1 = heap_alloc(heap, 8);
  heap_dump(heap);
  uint8_t *p2 = heap_alloc(heap, 3);
  heap_dump(heap);
  heap_free(heap, p1);
  heap_dump(heap);
  heap_free(heap, p2);
  heap_dump(heap);

  return 0;
}
*/
