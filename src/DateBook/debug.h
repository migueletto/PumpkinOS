#ifndef PIT_DEBUG_H
#define PIT_DEBUG_H

#ifdef __cplusplus  
extern "C" {
#endif

#define DEBUG_ERROR 0
#define DEBUG_INFO  1
#define DEBUG_TRACE 2

#ifdef PALMOS
#define debug_errno(sys, fmt, args...)
#define debugva(level, sys, fmt, args...)
#define debug(level, sys, fmt, args...)
#define debug_bytes(level, sys, buf, len)
#else
int debug_init(char *filename) EXTRA_SECTION_TWO;
int debug_close(void) EXTRA_SECTION_TWO;
void debug_errno_full(const char *file, const char *func, int line, const char *sys, const char *fmt, ...) EXTRA_SECTION_TWO;
void debugva_full(const char *file, const char *func, int line, int level, const char *sys, const char *fmt, va_list ap) EXTRA_SECTION_TWO;
void debug_full(const char *file, const char *func, int line, int level, const char *sys, const char *fmt, ...) EXTRA_SECTION_TWO;
void debug_bytes_full(const char *file, const char *func, int line, int level, const char *sys, unsigned char *buf, int len) EXTRA_SECTION_TWO;

#define debug_errno(sys, fmt, args...)    debug_errno_full(__FILE__, __FUNCTION__, __LINE__, sys, fmt, ##args)
#define debugva(level, sys, fmt, args...) debugva_full(__FILE__, __FUNCTION__, __LINE__, level, sys, fmt, ##args)
#define debug(level, sys, fmt, args...)   debug_full(__FILE__, __FUNCTION__, __LINE__, level, sys, fmt, ##args)
#define debug_bytes(level, sys, buf, len) debug_bytes_full(__FILE__, __FUNCTION__, __LINE__, level, sys, buf, len);
#endif

#ifdef __cplusplus
}
#endif

#endif
