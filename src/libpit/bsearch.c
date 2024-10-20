#include "sys.h"

void *sys_bsearch(const void *key, const void *base, sys_size_t nel, sys_size_t width, int (*compar)(const void *, const void *)) {
  void *try;
  int sign;

  while (nel > 0) {
    try = (char *)base + width*(nel/2);
    sign = compar(key, try);
    if (sign < 0) {
      nel /= 2;
    } else if (sign > 0) {
      base = (char *)try + width;
      nel -= nel/2+1;
    } else {
      return try;
    }
  } 
  
  return NULL;
}
