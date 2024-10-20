#include "sys.h"

int sys_sprintf(char *str, const char *format, ...) {
  sys_va_list ap;
  int r;
  
  sys_va_start(ap, format);
  r = sys_vsprintf(str, format, ap);
  sys_va_end(ap);
  
  return r;
}

int sys_snprintf(char *str, sys_size_t size, const char *format, ...) {
  sys_va_list ap;
  int r;

  sys_va_start(ap, format);
  r = sys_vsnprintf(str, size, format, ap);
  sys_va_end(ap);

  return r;
}
