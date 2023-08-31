//#define VISUAL_HEAP 1

#include "sys.h"
#include "heap.h"
#include "mutex.h"
#ifdef VISUAL_HEAP
#include "pwindow.h"
#include "script.h"
#include "media.h"
#include "pfont.h"
#include "graphic.h"
#include "surface.h"
#endif
#include "emulation/emupalmosinc.h"
#include "dlmalloc.h"
#include "debug.h"
#include "xalloc.h"

#define WIDTH  256
#define HEIGHT 256

#define HEAP_MARGIN 16*1024

struct heap_t {
  uint32_t size, pointer;
  uint8_t *start;
  uint8_t *end;
  void *state;
  mutex_t *mutex;
#ifdef VISUAL_HEAP
  window_provider_t *wp;
  window_t *w;
  texture_t *t;
  surface_t *surface;
  int x1, y1, x2, y2;
  int16_t bin[HEIGHT][WIDTH];
  uint64_t t0, last_render;
#endif
};

heap_t *heap_init(uint32_t size, void *_wp) {
  heap_t *heap;
#ifdef VISUAL_HEAP
  int width, height;
#endif

  debug(DEBUG_INFO, "Heap", "heap_init %u", size);

  if ((heap = xcalloc(1, sizeof(heap_t))) == NULL) {
    return NULL;
  }

  if ((heap->start = xcalloc(1, size)) == NULL) {
    xfree(heap);
    heap = NULL;
    return NULL;
  }

  heap->size = size;
  heap->end = (void *)(heap->start + size);
  heap->state = xcalloc(1, dlmalloc_state_size());

#ifdef VISUAL_HEAP
  width = WIDTH;
  height = HEIGHT;
  heap->wp = _wp;
  heap->w = heap->wp->create(ENC_RGBA, &width, &height, 2, 2, 0, 0, 0, heap->wp->data);
  heap->t = heap->wp->create_texture(heap->w, width, height);
  heap->surface = surface_create(width, height, SURFACE_ENCODING_ARGB);
  heap->t0 = sys_get_clock();
  heap->x1 = WIDTH;
  heap->y1 = HEIGHT;
  heap->x2 = -1;
  heap->y2 = -1;
#endif

  return heap;
}

void heap_finish(heap_t *heap) {
  if (heap) {
    debug(DEBUG_INFO, "Heap", "heap_finish");
    dlmalloc_stats();
    if (heap->start) xfree(heap->start);
    if (heap->state) xfree(heap->state);
#ifdef VISUAL_HEAP
    if (heap->surface) surface_destroy(heap->surface);
    if (heap->t) heap->wp->destroy_texture(heap->w, heap->t);
    if (heap->w) heap->wp->destroy(heap->w);
#endif
    xfree(heap);
  }
}

void *heap_base(heap_t *heap) {
  return heap->start;
}

uint32_t heap_size(heap_t *heap) {
  return heap->size;
}

#ifdef VISUAL_HEAP
static void heap_draw(heap_t *heap, uint8_t *from, uint8_t *to, int incr) {
  uint64_t t;
  uint32_t offset, color;
  uint16_t value;
  uint8_t *raw, *p;
  int len, x, y, red, green;

  for (p = from; p < to; p++) {
    offset = p - heap->start;
    offset >>= 9;
    x = offset & 0xFF;
    offset >>= 8;
    y = offset & 0xFF;

    if (y < heap->y1) heap->y1 = y;
    if (y > heap->y2) heap->y2 = y;
    if (x < heap->x1) heap->x1 = x;
    if (x > heap->x2) heap->x2 = x;

    heap->bin[y][x] += incr;
    if (heap->bin[y][x] < 0) {
      heap->bin[y][x] = 0;
    } else if (heap->bin[y][x] > 512) {
      heap->bin[y][x] = 512;
    }
    value = heap->bin[y][x];

    if (value < 256) {
      red = value;
      green = 0;
    } else if (value < 512) {
      red = 255;
      green = value - 256;
    } else {
      red = 255;
      green = 255;
    }

    color = heap->surface->color_rgb(heap->surface->data, red, green, 0, 255);
    heap->surface->setpixel(heap->surface->data, x, y, color);
  }

  if (heap->x2 >= heap->x1 && heap->y2 >= heap->y1) {
    t = sys_get_clock();
    if ((t - heap->last_render) >= 20000) {
      raw = (uint8_t *)heap->surface->getbuffer(heap->surface->data, &len);
      heap->wp->update_texture_rect(heap->w, heap->t, raw, heap->x1, heap->y1, heap->x2-heap->x1+1, heap->y2-heap->y1+1);
      heap->wp->draw_texture(heap->w, heap->t, 0, 0);
      heap->wp->render(heap->w);
      heap->x1 = WIDTH;
      heap->y1 = HEIGHT;
      heap->x2 = -1;
      heap->y2 = -1;
      heap->last_render = t;
    }
  }
}
#endif

void *heap_alloc(heap_t *heap, sys_size_t size) {
  void *p = dlmalloc(size);
  if (p) {
    sys_size_t *q = (sys_size_t *)p;
    sys_size_t realsize = (sys_size_t)(q[-1] & ~1);
    realsize -= 16;
    debug(DEBUG_TRACE, "Heap", "heap_alloc %u bytes %p to %p", realsize, p, (uint8_t *)p + realsize - 1);
#ifdef VISUAL_HEAP
    heap_draw(heap, p, (uint8_t *)p + realsize, 1);
#endif
  }
  return p;
}

void *heap_realloc(heap_t *heap, void *p, sys_size_t size) {
  if (p) {
    sys_size_t *q = (sys_size_t *)p;
    sys_size_t realsize = (sys_size_t)(q[-1] & ~1);
    realsize -= 16;
    debug(DEBUG_TRACE, "Heap", "heap_free %u bytes %p to %p", realsize, p, (uint8_t *)p + realsize - 1);
#ifdef VISUAL_HEAP
    heap_draw(heap, p, (uint8_t *)p + realsize, -1);
#endif

    p = dlrealloc(p, size);
    q = (sys_size_t *)p;
    realsize = (sys_size_t)(q[-1] & ~1);
    realsize -= 16;
    debug(DEBUG_TRACE, "Heap", "heap_alloc %u bytes %p to %p", realsize, p, (uint8_t *)p + realsize - 1);
#ifdef VISUAL_HEAP
    heap_draw(heap, p, (uint8_t *)p + realsize, 1);
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
    realsize -= 16;
    debug(DEBUG_TRACE, "Heap", "heap_free %u bytes %p to %p", realsize, p, (uint8_t *)p + realsize - 1);
#ifdef VISUAL_HEAP
    heap_draw(heap, p, (uint8_t *)p + realsize, -1);
#endif
    dlfree(p);
  }
}

void *dlmalloc_get_state(void) {
  heap_t *heap = heap_get();
  return heap->state;
}

void *heap_morecore(sys_size_t size) {
  heap_t *heap = heap_get();
  void *p = NULL;

  if ((heap->pointer + size) < heap->size - HEAP_MARGIN) {
    p = &heap->start[heap->pointer];
    heap->pointer += size;
    debug(DEBUG_TRACE, "Heap", "heap_morecore %u + %u < %u", heap->pointer, size, heap->size - HEAP_MARGIN);
  } else {
    debug(DEBUG_ERROR, "Heap", "heap_morecore %u + %u >= %u", heap->pointer, size, heap->size - HEAP_MARGIN);
    heap_exhausted_error();
  }

  return p;
}

void heap_dump(heap_t *heap) {
  int fd;

#ifdef VISUAL_HEAP
  surface_save(heap->surface, "heap.png", 0);
#endif

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

void heap_assert(const char *file, int line, const char *func, const char *cond) {
  char buf[256];

  // the heap is corrupted, reset it
  dlmalloc_init_state();

  if (func) {
    sys_snprintf(buf, sizeof(buf) - 1, "assert %s, %s, line %d: %s", file, func, line, cond);
  } else {
    sys_snprintf(buf, sizeof(buf) - 1, "assert %s, line %d: %s", file, line, cond);
  }
  debug(DEBUG_ERROR, "Heap", buf);

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
