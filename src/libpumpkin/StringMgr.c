#include <PalmOS.h>

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "pumpkin.h"
#include "debug.h"

Char *StrCopy(Char *dst, const Char *src) {
  if (dst == NULL || src == NULL) ErrFatalDisplayEx("StrCopy NULL", 0);
  if (dst && src) {
    strcpy(dst, src);
  }
  return dst;
}

Char *StrNCopy(Char *dst, const Char *src, Int16 n) {
  if (dst == NULL || src == NULL) ErrFatalDisplayEx("StrNCopy NULL", 0);
  if (dst && src) {
    strncpy(dst, src, n);
  }
  return dst;
}

Char *StrCat(Char *dst, const Char *src) {
  if (dst == NULL || src == NULL) ErrFatalDisplayEx("StrCat NULL", 0);
  if (dst && src) {
    strcat(dst, src);
  }
  return dst;
}

Char *StrNCat(Char *dst, const Char *src, Int16 n) {
  if (dst == NULL || src == NULL) ErrFatalDisplayEx("StrNCat NULL", 0);
  if (dst && src) {
    strncat(dst, src, n);
  }
  return dst;
}

UInt16 StrLen(const Char *src) {
  if (src == NULL) ErrFatalDisplayEx("StrLen NULL", 0);
  return src ? strlen(src) : 0;
}

Int16 StrCompareAscii(const Char *s1, const Char *s2) {
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("StrCompareAscii NULL", 0);
  return s1 && s2 ? strcmp(s1, s2) : -1;
}

Int16 StrCompare(const Char *s1, const Char *s2) {
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("StrCompare NULL", 0);
  return s1 && s2 ? strcmp(s1, s2) : -1;
}

Int16 StrNCompareAscii(const Char *s1, const Char *s2, Int32 n) {
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("StrNCompareAscii NULL", 0);
  return s1 && s2 ? strncmp(s1, s2, n) : -1;
}

Int16 StrNCompare(const Char *s1, const Char *s2, Int32 n) {
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("StrNCompare NULL", 0);
  return s1 && s2 ? strncmp(s1, s2, n) : -1;
}

Int16 StrCaselessCompare(const Char *s1, const Char *s2) {
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("StrCaselessCompare NULL", 0);
  return s1 && s2 ? strcasecmp(s1, s2) : -1;
}

Int16 StrNCaselessCompare(const Char *s1, const Char *s2, Int32 n) {
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("StrNCaselessCompare NULL", 0);
  return s1 && s2 ? strncasecmp(s1, s2, n) : -1;
}

Char *StrToLower(Char *dst, const Char *src) {
  int i;

  if (dst == NULL || src == NULL) ErrFatalDisplayEx("StrToLower NULL", 0);
  if (dst && src) {
    for (i = 0; src[i]; i++) {
      dst[i] = tolower(src[i]);
    }
  }

  return dst;
}

Char *StrIToA(Char *s, Int32 i) {
  if (s == NULL) ErrFatalDisplayEx("StrIToA NULL", 0);
  if (s) {
    sprintf(s, "%d", i);
  }
  return s;
}

Char *StrIToH(Char *s, UInt32 i) {
  if (s == NULL) ErrFatalDisplayEx("StrIToH NULL", 0);
  if (s) {
    sprintf(s, "%X", i);
  }
  return s;
}

Char *StrChr(const Char *str, WChar chr) {
  if (str == NULL) ErrFatalDisplayEx("StrChr NULL", 0);
  return str ? strchr(str, chr) : NULL;
}

Char *StrStr(const Char *str, const Char *token) {
  if (str == NULL || token == NULL) ErrFatalDisplayEx("StrStr NULL", 0);
  return str && token ? strstr(str, token) : NULL;
}

Int32 StrAToI(const Char *str) {
  if (str == NULL) ErrFatalDisplayEx("StrAToI NULL", 0);
  return str ? atoi(str) : 0;
}

Int16 StrPrintF(Char *s, const Char *formatStr, ...) {
  va_list ap;
  int n = 0;

  if (s == NULL || formatStr == NULL) ErrFatalDisplayEx("StrPrintF NULL", 0);
  if (s && formatStr) {
    va_start(ap, formatStr);
    n = vsprintf(s, formatStr, ap);
    va_end(ap);
  }

  return n;
}

Int16 StrVPrintF(Char *s, const Char *formatStr, _Palm_va_list arg) {
  int n = 0;

  if (s == NULL || formatStr == NULL) ErrFatalDisplayEx("StrVPrintF NULL", 0);
  if (s && formatStr) {
    n = vsprintf(s, formatStr, arg);
  }

  return n;
}

Char *StrLocalizeNumber(Char *s, Char thousandSeparator, Char decimalSeparator) {
  int i;

  if (s == NULL) ErrFatalDisplayEx("StrLocalizeNumber NULL", 0);
  if (s) {
    for (i = 0; s[i]; i++) {
      if (s[i] == ',') s[i] = thousandSeparator;
      else if (s[i] == '.') s[i] = decimalSeparator;
    }
  }

  return s;
}

Char *StrDelocalizeNumber(Char *s, Char thousandSeparator, Char decimalSeparator) {
  int i;

  if (s == NULL) ErrFatalDisplayEx("StrDelocalizeNumber NULL", 0);
  if (s) {
    for (i = 0; s[i]; i++) {
      if (s[i] == thousandSeparator) s[i] = ',';
      else if (s[i] == decimalSeparator) s[i] = '.';
    }
  }

  return s;
}
