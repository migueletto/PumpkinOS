#include <PalmOS.h>

#include "debug.h"

Err HwrCustom(UInt32 creator, UInt32 opCode, void *paramP, UInt16 *paramSizeP) {
  debug(DEBUG_ERROR, "PALMOS", "HwrCustom not implemented");
  return sysErrParamErr;
}
