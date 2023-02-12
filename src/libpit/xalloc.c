#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "xalloc.h"
#include "debug.h"

void *xmalloc_debug(const char *file, const char *func, int line, size_t size) {
  void *ptr = malloc(size);

  if (ptr) {
    debug_full(file, func, line, DEBUG_TRACE, "MEM", "memory new %p %d", ptr, size);
    memset(ptr, 0, size);
  } else {
    debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory new error %d", size);
  }

  return ptr;
}

void *xcalloc_debug(const char *file, const char *func, int line, size_t nmemb, size_t size) {
  size_t len = nmemb * size;
  return xmalloc_debug(file, func, line, len);
}

void *xrealloc_debug(const char *file, const char *func, int line, void *ptr, size_t size) {
  void *ptr2 = NULL;

  ptr2 = realloc(ptr, size);

  if (ptr2) {
    debug_full(file, func, line, DEBUG_TRACE, "MEM", "memory free %p", ptr);
    debug_full(file, func, line, DEBUG_TRACE, "MEM", "memory new %p %d", ptr2, size);
  } else {
    if (size) {
      debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory new error %d", size);
    } else {
      debug_full(file, func, line, DEBUG_TRACE, "MEM", "memory new %p %d", ptr2, size);
    }
  }

  return ptr2;
}

void xfree_debug(const char *file, const char *func, int line, void *ptr) {
  if (ptr) {
    debug_full(file, func, line, DEBUG_TRACE, "MEM", "memory free %p", ptr);
    free(ptr);
  } else {
    debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory free null");
  }
}

char *xstrdup_debug(const char *file, const char *func, int line, const char *s) {
  char *r = NULL;

  if (s) {
    r = strdup(s);

    if (r) {
      debug_full(file, func, line, DEBUG_TRACE, "MEM", "memory new %p %d", r, strlen(s)+1);
    } else {
      debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory new error %d", strlen(s)+1);
    }
  } else {
    debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory new null");
  }

  return r;
}

void *xmemcpy_debug(const char *file, const char *func, int line, void *dest, const void *src, size_t n) {
  void *r = NULL;

  if (dest) {
    //debug_full(file, func, line, DEBUG_TRACE, "MEM", "memory memcpy %08x %08x %d", dest, src ,n);
    r = memcpy(dest, src, n);
  } else {
    //debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory memcpy null");
  }

  return r;
}

void *xmemset_debug(const char *file, const char *func, int line, void *s, int c, size_t n) {
  void *r = NULL;

  if (s) {
    //debug_full(file, func, line, DEBUG_TRACE, "MEM", "memory memset %08x %d %d", s, c, n);
    r = memset(s, c, n);
  } else {
    //debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory memset null");
  }

  return r;
}
