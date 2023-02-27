#include "sys.h"
#include "xalloc.h"
#include "debug.h"

void *xmalloc_debug(const char *file, const char *func, int line, sys_size_t size) {
  void *ptr = sys_malloc(size);

  if (ptr) {
    debug_full(file, func, line, DEBUG_TRACE, "MEM", "memory new %p %d", ptr, size);
    sys_memset(ptr, 0, size);
  } else {
    debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory new error %d", size);
  }

  return ptr;
}

void *xcalloc_debug(const char *file, const char *func, int line, sys_size_t nmemb, sys_size_t size) {
  sys_size_t len = nmemb * size;
  return xmalloc_debug(file, func, line, len);
}

void *xrealloc_debug(const char *file, const char *func, int line, void *ptr, sys_size_t size) {
  void *ptr2 = NULL;

  ptr2 = sys_realloc(ptr, size);

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
    sys_free(ptr);
  } else {
    debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory free null");
  }
}

char *xstrdup_debug(const char *file, const char *func, int line, const char *s) {
  char *r = NULL;

  if (s) {
    r = sys_strdup(s);

    if (r) {
      debug_full(file, func, line, DEBUG_TRACE, "MEM", "memory new %p %p", s, r);
    } else {
      debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory new error %p", s);
    }
  } else {
    debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory new null");
  }

  return r;
}

void *xmemcpy_debug(const char *file, const char *func, int line, void *dest, const void *src, sys_size_t n) {
  void *r = NULL;

  if (dest) {
    debug_full(file, func, line, DEBUG_TRACE, "MEM", "memory memcpy %p %p %d", dest, src ,n);
    r = sys_memcpy(dest, src, n);
  } else {
    debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory memcpy null");
  }

  return r;
}

void *xmemset_debug(const char *file, const char *func, int line, void *s, int c, sys_size_t n) {
  void *r = NULL;

  if (s) {
    debug_full(file, func, line, DEBUG_TRACE, "MEM", "memory memset %p %d %d", s, c, n);
    r = sys_memset(s, c, n);
  } else {
    debug_full(file, func, line, DEBUG_ERROR, "MEM", "memory memset null");
  }

  return r;
}
