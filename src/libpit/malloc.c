#include "sys.h"

#if defined(KERNEL)

#include "dlm.h"

void *sys_malloc(sys_size_t size) {
  return dlm_malloc(NULL, size);
}

void sys_free(void *ptr) {
  dlm_free(NULL, ptr);
}

void *sys_calloc(sys_size_t nmemb, sys_size_t size) {
  void *p = dlm_malloc(NULL, nmemb * size);
  sys_memset(p, 0, nmemb * size);
  return p;
}

void *sys_realloc(void *ptr, sys_size_t size) {
  return dlm_realloc(NULL, ptr, size);
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
