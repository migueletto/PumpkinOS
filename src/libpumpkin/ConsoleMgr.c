#include <PalmOS.h>

#include "debug.h"

Err ConPutS(const Char *message) {
  debug(DEBUG_ERROR, "PALMOS", "ConPutS not implemented");
  return sysErrParamErr;
}

Err ConGetS(Char *message, Int32 timeout) {
  debug(DEBUG_ERROR, "PALMOS", "ConGetS not implemented");
  return sysErrParamErr;
}
