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

int plibc_iscntrl(int c) {
  return c < 32;
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

int plibc_isgraph(int c) {
  return c > 32 && c <= 255;
}

int plibc_ispunct(int c) {
  return plibc_isprint(c) && !plibc_isspace(c) && !plibc_isalnum(c);
}

int plibc_islower(int c) {
  return c >= 'a' && c <= 'z';
}

int plibc_isupper(int c) {
  return c >= 'A' && c <= 'Z';
}

int plibc_isalpha(int c) {
  return plibc_islower(c) || plibc_isupper(c);
}

int plibc_isdigit(int c) {
  return c >= '0' && c <= '9';
}

int plibc_isxdigit(int c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int plibc_isalnum(int c) {
  return plibc_isalpha(c) || plibc_isdigit(c);
}

int plibc_isprint(int c) {
  return plibc_isspace(c) || (c >= 32 && c <= 255);
}
