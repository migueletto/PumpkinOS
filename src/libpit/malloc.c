#include "sys.h"

#if defined(KERNEL)

#include "custom_malloc.h"

#define HEAP_SIZE 256 * 1024 * 1024
static uint8_t heap_buffer[HEAP_SIZE];
static HEAP_INFO_t heap;

void malloc_init(void) {
  heap.pHeap = heap_buffer;
  heap.heapSize = HEAP_SIZE;
  CustomMallocInit(&heap);
}

void *sys_malloc(sys_size_t size) {
  return CustomMalloc(&heap, size);
}

void sys_free(void *ptr) {
  CustomFree(&heap, ptr);
}

void *sys_calloc(sys_size_t nmemb, sys_size_t size) {
  void *p = sys_malloc(nmemb * size);
  sys_memset(p, 0, nmemb * size);
  return p;
}

void *sys_realloc(void *ptr, sys_size_t size) {
  BD_t *bd;
  void *p = NULL;

  if (ptr) {
    p = CustomMalloc(&heap, size);
    if (p) {
      bd = CustomBlock(&heap, ptr);
      sys_memcpy(p, ptr, size < bd->blkSize ? size : bd->blkSize);
    }
    CustomFree(&heap, ptr);
  }

  return p;
}

#else

#include <stdlib.h>

void *sys_malloc(sys_size_t size) {
  return malloc(size);
}

void sys_free(void *ptr) {
  free(ptr);
}

void *sys_calloc(sys_size_t nmemb, sys_size_t size) {
  return calloc(nmemb, size);
}

void *sys_realloc(void *ptr, sys_size_t size) {
  return realloc(ptr, size);
}
#endif
