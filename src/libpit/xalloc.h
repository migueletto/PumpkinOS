#ifndef PIT_XALLOC_H
#define PIT_XALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

void *xmalloc_debug(const char *file, const char *func, int line, size_t size);

void xfree_debug(const char *file, const char *func, int line, void *ptr);

void *xcalloc_debug(const char *file, const char *func, int line, size_t nmemb, size_t size);

void *xrealloc_debug(const char *file, const char *func, int line, void *ptr, size_t size);

char *xstrdup_debug(const char *file, const char *func, int line, const char *s);

void *
xmemcpy_debug(const char *file, const char *func, int line, void *dest, const void *src, size_t n);

void *xmemset_debug(const char *file, const char *func, int line, void *s, int c, size_t n);

#define xmalloc(size) xmalloc_debug(__FILE__, __FUNCTION__, __LINE__, size)
#define xfree(ptr) xfree_debug(__FILE__, __FUNCTION__, __LINE__, ptr)
#define xcalloc(nmemb, size) xcalloc_debug(__FILE__, __FUNCTION__, __LINE__, nmemb, size)
#define xrealloc(ptr, size) xrealloc_debug(__FILE__, __FUNCTION__, __LINE__, ptr, size)
#define xstrdup(s) xstrdup_debug(__FILE__, __FUNCTION__, __LINE__, s)
#define xmemcpy(dest, src, n) xmemcpy_debug(__FILE__, __FUNCTION__, __LINE__, dest, src, n)
#define xmemset(s, c, n) xmemset_debug(__FILE__, __FUNCTION__, __LINE__, s, c, n)

#ifdef __cplusplus
}
#endif

#endif
