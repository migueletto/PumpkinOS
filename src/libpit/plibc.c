#include "sys.h"
#include "plibc.h"

int plibc_sprintf(char *str, const char *format, ...) {
  sys_va_list ap;
  int r = 0;

  if (format) {
    sys_va_start(ap, format);
    r = sys_vsprintf(str, format, ap);
    sys_va_end(ap);
  }

  return r;
}

int plibc_snprintf(char *str, sys_size_t size, const char *format, ...) {
  sys_va_list ap;
  int r = 0;

  if (format) {
    sys_va_start(ap, format);
    r = sys_vsnprintf(str, size, format, ap);
    sys_va_end(ap);
  }

  return r;
}

int plibc_isspace(int c) {
  switch (c) {
    case ' ':
    case '\f':
    case '\n':
    case '\r':
    case '\t':
    case '\v': return 1;
  }
  return 0;
}

int plibc_isdigit(int c) {
  return c >= '0' && c <= '9';
}

int plibc_isprint(int c) {
  return plibc_isspace(c) || (c >= 32 && c <= 255);
}
