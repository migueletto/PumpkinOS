#include <PalmOS.h>

#include "sys.h"
#include "pumpkin.h"
#include "debug.h"

Char *StrCopy(Char *dst, const Char *src) {
  if (dst == NULL || src == NULL) ErrFatalDisplayEx("StrCopy NULL", 0);
  if (dst && src) {
    sys_strcpy(dst, src);
  }
  return dst;
}

Char *StrNCopy(Char *dst, const Char *src, Int16 n) {
  if (dst == NULL || src == NULL) ErrFatalDisplayEx("StrNCopy NULL", 0);
  if (dst && src) {
    sys_strncpy(dst, src, n);
  }
  return dst;
}

Char *StrCat(Char *dst, const Char *src) {
  if (dst == NULL || src == NULL) ErrFatalDisplayEx("StrCat NULL", 0);
  if (dst && src) {
    sys_strcat(dst, src);
  }
  return dst;
}

Char *StrNCat(Char *dst, const Char *src, Int16 n) {
  if (dst == NULL || src == NULL) ErrFatalDisplayEx("StrNCat NULL", 0);
  if (dst && src) {
    sys_strncat(dst, src, n);
  }
  return dst;
}

UInt16 StrLen(const Char *src) {
  if (src == NULL) ErrFatalDisplayEx("StrLen NULL", 0);
  return src ? sys_strlen(src) : 0;
}

Int16 StrCompareAscii(const Char *s1, const Char *s2) {
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("StrCompareAscii NULL", 0);
  return s1 && s2 ? sys_strcmp(s1, s2) : -1;
}

Int16 StrCompare(const Char *s1, const Char *s2) {
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("StrCompare NULL", 0);
  return s1 && s2 ? sys_strcmp(s1, s2) : -1;
}

Int16 StrNCompareAscii(const Char *s1, const Char *s2, Int32 n) {
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("StrNCompareAscii NULL", 0);
  return s1 && s2 ? sys_strncmp(s1, s2, n) : -1;
}

Int16 StrNCompare(const Char *s1, const Char *s2, Int32 n) {
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("StrNCompare NULL", 0);
  return s1 && s2 ? sys_strncmp(s1, s2, n) : -1;
}

Int16 StrCaselessCompare(const Char *s1, const Char *s2) {
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("StrCaselessCompare NULL", 0);
  return s1 && s2 ? sys_strcasecmp(s1, s2) : -1;
}

Int16 StrNCaselessCompare(const Char *s1, const Char *s2, Int32 n) {
  if (s1 == NULL || s2 == NULL) ErrFatalDisplayEx("StrNCaselessCompare NULL", 0);
  return s1 && s2 ? sys_strncasecmp(s1, s2, n) : -1;
}

Char *StrToLower(Char *dst, const Char *src) {
  int i;

  if (dst == NULL || src == NULL) ErrFatalDisplayEx("StrToLower NULL", 0);
  if (dst && src) {
    for (i = 0; src[i]; i++) {
      dst[i] = sys_tolower(src[i]);
    }
  }

  return dst;
}

Char *StrIToA(Char *s, Int32 i) {
  if (s == NULL) ErrFatalDisplayEx("StrIToA NULL", 0);
  if (s) {
    sys_snprintf(s, 16, "%d", i);
  }
  return s;
}

Char *StrIToH(Char *s, UInt32 i) {
  if (s == NULL) ErrFatalDisplayEx("StrIToH NULL", 0);
  if (s) {
    sys_snprintf(s, 8, "%X", i);
  }
  return s;
}

Char *StrChr(const Char *str, WChar chr) {
  if (str == NULL) ErrFatalDisplayEx("StrChr NULL", 0);
  return str ? sys_strchr(str, chr) : NULL;
}

Char *StrStr(const Char *str, const Char *token) {
  if (str == NULL || token == NULL) ErrFatalDisplayEx("StrStr NULL", 0);
  return str && token ? sys_strstr(str, token) : NULL;
}

Int32 StrAToI(const Char *str) {
  if (str == NULL) ErrFatalDisplayEx("StrAToI NULL", 0);
  return str ? sys_atoi(str) : 0;
}

Int16 StrNPrintF(Char *s, UInt16 size, const Char *formatStr, ...) {
  sys_va_list ap;
  int n = 0;

  if (s == NULL || formatStr == NULL) ErrFatalDisplayEx("StrNPrintF NULL", 0);
  if (s && formatStr) {
    sys_va_start(ap, formatStr);
    n = sys_vsnprintf(s, size, formatStr, ap);
    sys_va_end(ap);
  }

  return n;
}

Int16 StrPrintF(Char *s, const Char *formatStr, ...) {
  sys_va_list ap;
  int n = 0;

  if (s == NULL || formatStr == NULL) ErrFatalDisplayEx("StrPrintF NULL", 0);
  if (s && formatStr) {
    sys_va_start(ap, formatStr);
    n = sys_vsprintf(s, formatStr, ap);
    sys_va_end(ap);
  }

  return n;
}

Int16 StrVNPrintF(Char *s, UInt16 size, const Char *formatStr, sys_va_list arg) {
  int n = 0;

  if (s == NULL || formatStr == NULL) ErrFatalDisplayEx("StrVNPrintF NULL", 0);
  if (s && formatStr) {
    n = sys_vsnprintf(s, size, formatStr, arg);
  }

  return n;
}

Int16 StrVPrintF(Char *s, const Char *formatStr, sys_va_list arg) {
  int n = 0;

  if (s == NULL || formatStr == NULL) ErrFatalDisplayEx("StrVPrintF NULL", 0);
  if (s && formatStr) {
    n = sys_vsnprintf(s, 256, formatStr, arg);
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
