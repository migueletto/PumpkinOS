#if !defined(KERNEL)
#include <stdio.h>
#endif

#if defined(ANDROID)
#include <android/log.h>
#endif

#include "sys.h"
#include "thread.h"
#include "mutex.h"
#include "timeutc.h"
#include "debug.h"

#define MAX_BUF 1024
#define MAX_SYS 32

typedef struct {
  char *sys;
  int level;
} debug_sys_t;

#if !defined(KERNEL)
static FILE *fd = NULL;
static mutex_t *mutex;
#endif
static int level = DEBUG_ERROR;
static debug_sys_t sys_level[MAX_SYS];
static int nlevels = 0;
static int show_scope = 0;
static int indent = 0;
static int raw = 0;
static int inited = 0;

static char level_name[] = { 'E', 'I', 'T' };

int debug_init(char *filename) {
#if !defined(KERNEL)
  mutex = mutex_create("debug");
  if (filename) {
    if (!sys_strcmp(filename, "stdout")) fd = stdout;
    else if (!sys_strcmp(filename, "stderr")) fd = stderr;
    else fd = fopen(filename, "w");
  } else {
    fd = stderr;
  }
  if (fd == NULL) fd = stderr;
#endif
  inited = 1;
  return 0;
}

int debug_close(void) {
#if !defined(KERNEL)
  if (fd && fd != stderr && fd != stdout) {
    fclose(fd);
    fd = NULL;
  }
  mutex_destroy(mutex);
#endif

  return 0;
}

void debug_aindent(int i) {
  indent = i;
}

void debug_indent(int incr) {
  indent += incr;
}

void debug_rawtty(int _raw) {
  raw = _raw;
}

void debug_setsyslevel(char *sys, int _level) {
  int i;

  if (_level < DEBUG_ERROR) _level = DEBUG_ERROR;
  if (_level > DEBUG_TRACE) _level = DEBUG_TRACE;

  if (sys) {
    for (i = 0; i < nlevels; i++) {
      if (sys_level[i].sys != NULL) {
        if (!sys_strcmp(sys_level[i].sys, sys)) {
          sys_level[i].level = _level;
          return;
        }
      }
    }

    if (i < MAX_SYS) {
      sys_level[i].sys = sys;
      sys_level[i].level = _level;
      nlevels = i+1;
    }
  } else {
    level = _level;
  }
}

int debug_getsyslevel(char *sys) {
  int i;

  if (sys) {
    for (i = 0; i < nlevels; i++) {
      if (sys_level[i].sys != NULL) {
        if (!sys_strcmp(sys_level[i].sys, sys)) {
          return sys_level[i].level;
        }
      }
    }
  }

  return level;
}

void debug_scope(int show) {
  show_scope = show;
}

static char hex(uint8_t n) {
  n &= 0x0F;
  return n < 10 ? '0' + n : 'A' + n - 10;
}

static int dec(uint32_t n, int d, char *s, int max) {
  int i;

  for (i = 0; i < d && i < max; i++) {
    s[d-i-1] = '0' + (n % 10);
    n /= 10;
  }

  return i;
}

int ch(char c, char *s, int max) {
  if (max <= 0) return 0;
  s[0] = c;
  return 1;
}

int str(char *buf, int d, char *s, int max) {
  int i;

  for (i = 0; buf[i] && i < max; i++) {
    if (i == d) break;
    s[i] = buf[i];
  }
  for (; i < d && i < max; i++) {
    s[i] = ' ';
  }

  return i;
}

#ifndef MUTE_DEBUG
void debugva_full(const char *file, const char *func, int line, int _level, const char *sys, const char *fmt, sys_va_list ap) {
  char tmp[MAX_BUF], buf[MAX_BUF], *s;
  int i, j, k, us;
  char thread_name[32];
  sys_timeval_t tv;
  sys_tm_t tm;
  uint64_t ts;

  if (!inited) return;
  if (_level < DEBUG_ERROR) _level = DEBUG_ERROR;
  if (_level > DEBUG_TRACE) _level = DEBUG_TRACE;

  if (_level <= debug_getsyslevel((char *)sys)) {
    sys_vsnprintf(tmp, sizeof(tmp)-1, fmt, ap);

    for (i = 0, j = 0; tmp[i] && j < MAX_BUF-5; i++) {
      if (tmp[i] >= 32) {
        buf[j++] = tmp[i];
      } else if (tmp[i+1]) {
        buf[j++] = '<';
        buf[j++] = hex((tmp[i] >> 4) & 0x0F);
        buf[j++] = hex(tmp[i] & 0x0F);
        buf[j++] = '>';
      }
    }
    buf[j] = 0;

    thread_get_name(thread_name, sizeof(thread_name));

    sys_timeofday(&tv);

    ts = tv.tv_sec;
    us = tv.tv_usec;
    utctime(&ts, &tm);

    s = tmp;
    s += dec(tm.tm_year + 1900, 4, s, tmp + MAX_BUF - s);
    s += ch('-', s, tmp + MAX_BUF - s);
    s += dec(tm.tm_mon + 1, 2, s, tmp + MAX_BUF - s);
    s += ch('-', s, tmp + MAX_BUF - s);
    s += dec(tm.tm_mday, 2, s, tmp + MAX_BUF - s);
    s += ch(' ', s, tmp + MAX_BUF - s);
    s += dec(tm.tm_hour, 2, s, tmp + MAX_BUF - s);
    s += ch(':', s, tmp + MAX_BUF - s);
    s += dec(tm.tm_min, 2, s, tmp + MAX_BUF - s);
    s += ch(':', s, tmp + MAX_BUF - s);
    s += dec(tm.tm_sec, 2, s, tmp + MAX_BUF - s);
    s += ch('.', s, tmp + MAX_BUF - s);
    s += dec(us, 6, s, tmp + MAX_BUF - s);
    s += ch(' ', s, tmp + MAX_BUF - s);
    s += ch(level_name[_level], s, tmp + MAX_BUF - s);
    s += ch(' ', s, tmp + MAX_BUF - s);
    s += dec(sys_get_tid(), 5, s, tmp + MAX_BUF - s);
    s += ch(' ', s, tmp + MAX_BUF - s);
    s += str(thread_name, 8, s, tmp + MAX_BUF - s);
    s += ch(' ', s, tmp + MAX_BUF - s);
    s += str((char *)sys, -1, s, tmp + MAX_BUF - s);
    s += ch(':', s, tmp + MAX_BUF - s);
    s += ch(' ', s, tmp + MAX_BUF - s);
    for (k = 0; k < indent; k++) {
      s += ch(' ', s, tmp + MAX_BUF - s);
    }
    s += str(buf, -1, s, tmp + MAX_BUF - s);
    if (show_scope) {
      s += ch(' ', s, tmp + MAX_BUF - s);
      s += ch('[', s, tmp + MAX_BUF - s);
      s += str((char *)file, -1, s, tmp + MAX_BUF - s);
      s += ch(':', s, tmp + MAX_BUF - s);
      s += str((char *)func, -1, s, tmp + MAX_BUF - s);
      s += ch(':', s, tmp + MAX_BUF - s);
      s += dec(line, 4, s, tmp + MAX_BUF - s);
      s += ch(']', s, tmp + MAX_BUF - s);
    }
    if (raw) s += ch('\r', s, tmp + MAX_BUF - s);
    s += ch('\n', s, tmp + MAX_BUF - s);
    *s = 0;

#if defined(ANDROID)
    switch (_level) {
      case DEBUG_TRACE: _level = ANDROID_LOG_INFO; break;
      case DEBUG_INFO:  _level = ANDROID_LOG_INFO; break;
      case DEBUG_ERROR: _level = ANDROID_LOG_ERROR; break;
      default: _level = ANDROID_LOG_INFO;
    }
    __android_log_buf_write(LOG_ID_MAIN, _level, "pit", tmp);
#elif !defined(KERNEL)
    mutex_lock_only(mutex);
    fwrite((uint8_t *)tmp, 1, s - tmp, fd);
    if (s > tmp && tmp[s - tmp - 1] != '\n') {
      if (raw) fwrite("\r", 1, 1, fd);
      fwrite("\n", 1, 1, fd);
    }
    fflush(fd);
    mutex_unlock_only(mutex);
#endif
  }
}

void debug_full(const char *file, const char *func, int line, int _level, const char *sys, const char *fmt, ...) {
  sys_va_list ap;

  if (!inited) return;
  sys_va_start(ap, fmt);
  debugva_full(file, func, line, _level, sys, fmt, ap);
  sys_va_end(ap);
}

void debug_errno_full(const char *file, const char *func, int line, const char *sys, const char *fmt, ...) {
  sys_va_list ap;
  char buf[MAX_BUF], msg[MAX_BUF];
  int err;

  if (!inited) return;
  err = sys_errno();
  sys_strerror(err, msg, sizeof(msg));

  sys_va_start(ap, fmt);
  sys_vsnprintf(buf, sizeof(buf)-1, fmt, ap);
  sys_va_end(ap);

  debug_full(file, func, line, DEBUG_ERROR, sys, "%s failed: %d (%s)", buf, err, msg);
}

void debug_bytes_offset_full(const char *file, const char *func, int line, int level, const char *sys, unsigned char *buf, int len, unsigned int offset) {
  char sbuf[1024], abuf[32], *p, *e;
  uint32_t i, j, n;

  if (!inited) return;
  p = sbuf;
  sys_sprintf(p, "%08X: ", offset);
  n = sys_strlen(p);
  p += n;
  e = p + 1024 - n - 4;

  for (i = 0, j = 0; i < len && p < e; i++) {
    if (j) {
      *p = ' ';
      p++;
    }
    sys_sprintf(p, "%02X", buf[i]);
    abuf[j] = (buf[i] >= 32 && buf[i] < 127) ? buf[i] : '.';
    p += 2;
    j++;
    if (j == 16) {
      *p = 0;
      abuf[j] = 0;
      debug_full(file, func, line, level, sys, "%s %s", sbuf, abuf);
      p = sbuf;
      sys_sprintf(p, "%08X: ", offset+i+1);
      n = sys_strlen(p);
      p += n;
      e = p + 1024 - n - 4;
      j = 0;
    }
  }
  *p = 0;
  abuf[j] = 0;

  if (j) {
    for (; j < 16; j++) {
      *p++ = ' ';
      *p++ = ' ';
      *p++ = ' ';
      *p = 0;
    }
    debug_full(file, func, line, level, sys, "%s %s", sbuf, abuf);
  }
}

void debug_bytes_full(const char *file, const char *func, int line, int level, const char *sys, unsigned char *buf, int len) {
  debug_bytes_offset_full(file, func, line, level, sys, buf, len, 0);
}
#endif
