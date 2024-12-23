#ifndef PIT_DEBUG_H
#define PIT_DEBUG_H

#ifdef __cplusplus  
extern "C" {
#endif

#define DEBUG_ERROR 0
#define DEBUG_INFO  1
#define DEBUG_TRACE 2

int debug_init(char *filename);

int debug_close(void);

void debug_setsyslevel(char *sys, int level);

int debug_getsyslevel(char *sys);

void debug_scope(int show);

void debug_aindent(int i);

void debug_indent(int incr);

void debug_rawtty(int raw);

void
debug_errno_full(const char *file, const char *func, int line, const char *sys, const char *fmt,
                 ...) __attribute__ ((format (printf, 5, 6)));

void debugva_full(const char *file, const char *func, int line, int level, const char *sys,
                  const char *fmt, sys_va_list ap);

void debug_full(const char *file, const char *func, int line, int level, const char *sys,
                const char *fmt, ...) __attribute__ ((format (printf, 6, 7)));

void debug_bytes_full(const char *file, const char *func, int line, int level, const char *sys,
                      unsigned char *buf, int len);

void debug_bytes_offset_full(const char *file, const char *func, int line, int level, const char *sys,
                             unsigned char *buf, int len, unsigned int offset);

#define debug_errno(sys, fmt, args...)    debug_errno_full(__FILE__, __FUNCTION__, __LINE__, sys, fmt, ##args)
#define debugva(level, sys, fmt, args...) debugva_full(__FILE__, __FUNCTION__, __LINE__, level, sys, fmt, ##args)
#define debug(level, sys, fmt, args...)   debug_full(__FILE__, __FUNCTION__, __LINE__, level, sys, fmt, ##args)
#define debug_bytes(level, sys, buf, len) debug_bytes_full(__FILE__, __FUNCTION__, __LINE__, level, sys, buf, len);
#define debug_bytes_offset(level, sys, buf, len, offset) debug_bytes_offset_full(__FILE__, __FUNCTION__, __LINE__, level, sys, buf, len, offset);

#ifdef __cplusplus
}
#endif

#endif
