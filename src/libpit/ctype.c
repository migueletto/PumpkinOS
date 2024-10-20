#include "sys.h"

int sys_toupper(int c) {
  if (sys_islower(c)) return c & 0x5f;
  return c;
}

int sys_tolower(int c) {
  if (sys_isupper(c)) return c | 32;
  return c;
}

int sys_iscntrl(int c) {
  return c < 32;
}

int sys_isblank(int c) {
  return c == ' ' || c == '\t';
}

int sys_isspace(int c) {
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

int sys_isgraph(int c) {
  return c > 32 && c <= 255;
}

int sys_ispunct(int c) {
  return sys_isprint(c) && !sys_isspace(c) && !sys_isalnum(c);
}

int sys_islower(int c) {
  return c >= 'a' && c <= 'z';
}

int sys_isupper(int c) {
  return c >= 'A' && c <= 'Z';
}

int sys_isalpha(int c) {
  return sys_islower(c) || sys_isupper(c);
}

int sys_isdigit(int c) {
  return c >= '0' && c <= '9';
}

int sys_isxdigit(int c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int sys_isalnum(int c) {
  return sys_isalpha(c) || sys_isdigit(c);
}

int sys_isprint(int c) {
  return sys_isspace(c) || (c >= 32 && c <= 255);
}
