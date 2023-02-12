#include <PalmOS.h>

#include "debug.h"

void *IntlGetRoutineAddress(IntlSelector inSelector) {
  debug(DEBUG_ERROR, "PALMOS", "IntlGetRoutineAddress not implemented");
  return NULL;
}

Err IntlSetRoutineAddress(IntlSelector iSelector, void* iProcPtr) {
  debug(DEBUG_ERROR, "PALMOS", "IntlSetRoutineAddress not implemented");
  return sysErrParamErr;
}
