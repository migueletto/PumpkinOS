#include <PalmOS.h>

#include "sys.h"
#include "trig.h"

double *sin_table;
double *cos_table;

void TrigInit(void) {
  double pi;
  int i;

  sin_table = (double *)MemPtrNew(91 * sizeof(double));
  cos_table = (double *)MemPtrNew(91 * sizeof(double));
  pi = sys_pi();

  for (i = 0; i <= 90; i++) {
    sin_table[i] = sys_sin((i * pi) / 180.0);
    cos_table[i] = sys_cos((i * pi) / 180.0);
  }
}

void TrigFinish(void) {
  MemPtrFree(sin_table);
  MemPtrFree(cos_table);
}
