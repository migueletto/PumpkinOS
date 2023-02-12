#include <PalmOS.h>

#include "sec.h"
#include "SysDebug.h"

#ifdef PALMOS
#include <stdarg.h>
#else
#include "debug.h"
#endif

void SysDebug(UInt16 level, Char *sys, Char *fmt, ...) {
  va_list ap;
  char buf[256];
#ifndef PALMOS
  char format[256];
  char *s;
#endif

#ifdef PALMOS
  va_start(ap, fmt);
  StrVPrintF(buf, fmt, ap);
  va_end(ap);
  PumpkinDebug(level, sys, buf);
#else
  StrNCopy(format, fmt, sizeof(format)-1);
  s = format;
  for (;;) {
    s = StrStr(s, "%ld");
    if (!s) break;
    s[1] = 'd';
    s[2] = ' ';
    s += 3;
  }
  s = format;
  for (;;) {
    s = StrStr(s, "%08lX");
    if (!s) break;
    s[3] = 'X';
    s[4] = ' ';
    s += 5;
  }
  va_start(ap, fmt);
  StrVPrintF(buf, format, ap);
  va_end(ap);
  debug(level, sys, "%s", buf);
#endif
}

void SysDebugBytes(UInt16 level, Char *sys, void *buf, UInt32 len) {
#ifdef PALMOS
  PumpkinDebugBytes(level, sys, buf, len);
#else
  debug_bytes(level, sys, buf, len);
#endif
}
